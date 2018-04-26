#ifndef MODEL_EXTENSIONS_HPP
#define MODEL_EXTENSIONS_HPP

#include <string>
#include <memory>

#include <Model.hpp>
#include <SVG.hpp>
#include <BoundingBox.hpp>

namespace bp2d {

using SvgDoc = Slic3r::SVG;

using SvgPtr = std::unique_ptr<SvgDoc>;

SvgPtr svgFromModel(const Slic3r::Model& model,
                    const std::string& file_path = "");

void exportSVG(Slic3r::Model& model,
               coordf_t dist,
               const Slic3r::BoundingBoxf* bb);
//void exportSVG(Arranger::PackGroup& result, const Arranger::BinType& bin);


std::vector<ClipperLib::PolyNode> modelSiluett(const Slic3r::Model &model);

}

#endif // MODEL_EXTENSIONS_HPP
