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
               const Slic3r::BoundingBoxf* bb,
               const double downscale);

/**
 * @brief Arrange model objects to best fit onto the print bed.
 *
 * Objects which don't fit onto the print bed will be placed outside near the
 * print bed. All objects will be arranged so that they can be moved to the
 * print bed as a group without modifying the relative positions.
 *
 * @param model The 3D print model with all the object instances.
 * @param dist The allowed minimum object distance. No object can be closer to
 * another object than this value.
 * @param bb The print bed shape.
 */
void arrange(Slic3r::Model& model,
             coordf_t dist,
             const Slic3r::BoundingBoxf& bb);

}

#endif // MODEL_EXTENSIONS_HPP
