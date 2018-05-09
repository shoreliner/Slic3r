#include "ModelExtensions.hpp"

#include <binpack2d.h>
#include <binpack2d/geometries_io.hpp>

#include <wx/stdpaths.h>
#include <wx/filedlg.h>
#include <wx/app.h>

#include <fstream>
#include <numeric>

#include <ClipperUtils.hpp>
#include "slic3r/GUI/GUI.hpp"

using namespace Slic3r;

namespace bp2d {

using namespace binpack2d;

SvgPtr svgFromModel(const Slic3r::Model &model, const std::string& file_path)
{
    std::string svgdir = file_path.empty() ? wxStandardPaths::Get().GetTempDir()
                                           : file_path ;

    std::unique_ptr<SvgDoc> svg( new SvgDoc(svgdir.c_str(),
                                            SvgDoc::WITHOUT_MARKER));

    for(auto objptr : model.objects) {
        if(objptr) {

            auto rmesh = objptr->raw_mesh();

            for(auto objinst : objptr->instances) {
                if(objinst) {
                    Slic3r::TriangleMesh tmpmesh = rmesh;
                    objinst->transform_mesh(&tmpmesh);
                    auto chull = tmpmesh.convex_hull();

                    // draw bb to svg
                    svg->draw(chull);
                }
            }
        }
    }

    return svg;
}

// A container which stores a pointer to the 3D object and its projected
// 2D shape from top view.
using ShapeData2D =
    std::vector<std::pair<Slic3r::ModelInstance*, Item>>;

ShapeData2D projectModelFromTop(const Slic3r::Model &model) {
    ShapeData2D ret;

    auto s = std::accumulate(model.objects.begin(), model.objects.end(), 0,
                    [](size_t s, ModelObject* o){
        return s + o->instances.size();
    });

    ret.reserve(s);

    for(auto objptr : model.objects) {
        if(objptr) {

            auto rmesh = objptr->raw_mesh();

            for(auto objinst : objptr->instances) {
                if(objinst) {
                    Slic3r::TriangleMesh tmpmesh = rmesh;
                    objinst->transform_mesh(&tmpmesh);
                    ClipperLib::PolyNode pn;
                    auto p = tmpmesh.convex_hull();
                    p.make_clockwise();
                    p.append(p.first_point());
                    pn.Contour = Slic3rMultiPoint_to_ClipperPath( p );

                    ret.emplace_back(objinst, Item(std::move(pn)));
                }
            }
        }
    }

    return ret;
}

void exportSVG(Slic3r::Model& model,
               coordf_t dist,
               const Slic3r::BoundingBoxf* bb, const double downscale)
{
    wxFileDialog saveFileDialog(GUI::get_app()->GetTopWindow(),
                                _("Save svg file for svgnest"), "", "",
                                "svg files (*.svg)|*.svg",
                                wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    Box bin({
                static_cast<binpack2d::Coord>(bb->min.x),
                static_cast<binpack2d::Coord>(bb->min.y)
            },
            {
                static_cast<binpack2d::Coord>(bb->max.x),
                static_cast<binpack2d::Coord>(bb->max.y)
            });

    auto shapemap = bp2d::projectModelFromTop(model);

    std::vector<std::reference_wrapper<Item>> shapes;
    shapes.reserve(shapemap.size());

    std::for_each(shapemap.begin(), shapemap.end(),
                  [&shapes](ShapeData2D::value_type& it){
       shapes.push_back(std::ref(it.second));
    });

    Arranger<BottomLeftPlacer, DJDHeuristic>
            arrange(bin, static_cast<Coord>(dist)/downscale);

    auto result = arrange(shapes.begin(), shapes.end());

    auto loc = saveFileDialog.GetPath().ToStdString();

    static std::string svg_header =
    R"raw(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
    <!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.0//EN" "http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">
    <svg height="500" width="500" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
    )raw";

    int i = 0;
    for(auto r : result) {
        std::fstream out(loc + std::to_string(i) + ".svg", std::fstream::out);
        if(out.is_open()) {

            out << svg_header;

            Item rbin( bp2d::Rectangle(bin.width(), bin.height()) );

            for(unsigned j = 0; j < rbin.vertexCount(); j++) {
                auto v = rbin.vertex(j);
                setY(v, -getY(v)/downscale + 500 );
                setX(v, getX(v)/downscale);
                rbin.setVertex(j, v);
            }

            out << ShapeLike::serialize<Formats::SVG>(rbin.rawShape()) << std::endl;

            for(auto& sh : r) {
                Item tsh(sh.get().transformedShape());
                for(unsigned j = 0; j < tsh.vertexCount(); j++) {
                    auto v = tsh.vertex(j);
                    setY(v, -getY(v)/downscale + 500);
                    setX(v, getX(v)/downscale);
                    tsh.setVertex(j, v);
                }
                out << ShapeLike::serialize<Formats::SVG>(tsh.rawShape()) << std::endl;
            }

            out << "\n</svg>" << std::endl;
        }
        out.close();

        i++;
    }
}


bool arrange(Model &model, coordf_t dist, const Slic3r::BoundingBoxf* bb,
             bool first_bin_only)
{
    using ArrangeResult = _IndexedPackGroup<PolygonImpl>;

    bool ret = true;

    // Create the arranger config
    auto min_obj_distance = static_cast<Coord>(dist/SCALING_FACTOR);

    // Get the 2D projected shapes with their 3D model instance pointers
    auto shapemap = bp2d::projectModelFromTop(model);

    double area = 0;
    double area_max = 0;
    Item *biggest = nullptr;

    // Copy the references for the shapes only as the arranger expects a
    // sequence of objects convertible to Item or ClipperPolygon
    std::vector<std::reference_wrapper<Item>> shapes;
    shapes.reserve(shapemap.size());
    std::for_each(shapemap.begin(), shapemap.end(),
                  [&shapes, &area, min_obj_distance, &area_max, &biggest]
                  (ShapeData2D::value_type& it)
    {
        Item& item = it.second;
        item.addOffset(min_obj_distance);
        auto b = ShapeLike::boundingBox(item.transformedShape());
        auto a = b.width()*b.height();
        if(area_max < a) {
            area_max = a;
            biggest = &item;
        }
        area += b.width()*b.height();
        shapes.push_back(std::ref(it.second));
    });

    Box bin;

    if(bb != nullptr && bb->defined) {
        // Scale up the bounding box to clipper scale.
        BoundingBoxf bbb = *bb;
        bbb.scale(1.0/SCALING_FACTOR);

        bin = Box({
                    static_cast<binpack2d::Coord>(bbb.min.x),
                    static_cast<binpack2d::Coord>(bbb.min.y)
                },
                {
                    static_cast<binpack2d::Coord>(bbb.max.x),
                    static_cast<binpack2d::Coord>(bbb.max.y)
                });
    } else {
        // Just take the biggest item as bin... ?
        bin = ShapeLike::boundingBox(biggest->transformedShape());
    }

    // Will use the DJD selection heuristic with the BottomLeft placement
    // strategy
    using Arranger = Arranger<BottomLeftPlacer, DJDHeuristic>;

    Arranger arranger(bin, min_obj_distance);

    // Arrange and return the items with their respective indices within the
    // input sequence.
    Arranger::IndexedPackGroup result =
            arranger.arrangeIndexed(shapes.begin(), shapes.end());


    auto applyResult = [&shapemap](ArrangeResult::value_type& group,
            Coord batch_offset)
    {
        for(auto& r : group) {
            auto idx = r.first;     // get the original item index
            Item& item = r.second;  // get the item itself

            // Get the model instance from the shapemap using the index
            ModelInstance *inst_ptr = shapemap[idx].first;

            // Get the tranformation data from the item object and scale it
            // appropriately
            Radians rot = item.rotation();
            auto off = item.translation();
            Pointf foff(off.X*SCALING_FACTOR + batch_offset,
                        off.Y*SCALING_FACTOR);

            // write the tranformation data into the model instance
            inst_ptr->rotation += rot;
            inst_ptr->offset += foff;

            // Debug
            /*std::cout << "item " << idx << ": \n" << "\toffset_x: "
             * << foff.x << "\n\toffset_y: " << foff.y << std::endl;*/
        }
    };

    if(first_bin_only) {
        applyResult(result.front(), 0);
    } else {
        auto batch_offset = 0;
        for(auto& group : result) {
            applyResult(group, batch_offset);

            // Only the first pack group can be placed onto the print bed. The
            // other objects which could not fit will be placed next to the
            // print bed
            batch_offset += 2*bin.width()*SCALING_FACTOR;
        }
    }

    for(auto objptr : model.objects) objptr->invalidate_bounding_box();

    return ret && result.size() == 1;
}

}
