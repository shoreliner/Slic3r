#include "ModelExtensions.hpp"

#include <binpack2d.h>
#include <binpack2d/geometries_io.hpp>

#include <wx/stdpaths.h>
#include <wx/filedlg.h>
#include <wx/app.h>
#include <fstream>

#include <ClipperUtils.hpp>
#include "slic3r/GUI/GUI.hpp"

using namespace Slic3r;

namespace bp2d {

using namespace binpack2d;

SvgPtr svgFromModel(const Slic3r::Model &model, const std::string& file_path)
{
    std::string svgdir = file_path.empty() ? wxStandardPaths::Get().GetTempDir() : file_path ;

    SvgPtr svg = std::make_unique<SvgDoc>(svgdir.c_str(), SvgDoc::WITHOUT_MARKER);

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

std::vector<ClipperLib::PolyNode> modelSiluett(const Slic3r::Model &model) {
    std::vector<ClipperLib::PolyNode> ret;
    for(auto objptr : model.objects) {
        if(objptr) {

            auto rmesh = objptr->raw_mesh();

            for(auto objinst : objptr->instances) {
                if(objinst) {
                    Slic3r::TriangleMesh tmpmesh = rmesh;
                    objinst->transform_mesh(&tmpmesh);
                    ClipperLib::PolyNode pn;
                    auto p = tmpmesh._convex_hull();
                    p.make_clockwise();
                    p.append(p.first_point());
                    pn.Contour = Slic3rMultiPoint_to_ClipperPath( p );

                    ret.push_back(pn);
                }
            }
        }
    }

    return ret;
}

void exportSVG(Slic3r::Model& model,
               coordf_t dist,
               const Slic3r::BoundingBoxf* bb)
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

    auto shapes = bp2d::modelSiluett(model);

    Arranger::PlacementConfig config;
        config.min_obj_distance = static_cast<Coord>(dist);

    Arranger arrange(bin, config);

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

            binpack2d::Rectangle rbin(bin.width(), bin.height());

            for(auto&v : rbin) setY(v, -getY(v) + 500 );

            out << ShapeLike::serialize<Formats::SVG>(rbin.rawShape()) << std::endl;

            for(auto& sh : r) {
                Item tsh = sh.get().transformedShape();
                for(auto&v : tsh) setY(v, -getY(v) + 500 );
                out << ShapeLike::serialize<Formats::SVG>(tsh.rawShape()) << std::endl;
            }

            out << "\n</svg>" << std::endl;
        }
        out.close();

        i++;
    }
}

}
