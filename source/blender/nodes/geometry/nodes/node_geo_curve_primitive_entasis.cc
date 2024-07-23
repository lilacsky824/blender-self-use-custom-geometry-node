/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "BKE_curves.hh"
#include "BLI_math_matrix.hh"

#include "GEO_transform.hh"

#include "UI_interface.hh"
#include "UI_resources.hh"

#include "node_geometry_util.hh"

namespace blender::nodes::node_geo_curve_primitive_entasis_cc {

#define MAX_N 16
static void node_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::Int>("N")
      .default_value(2)
      .min(1)
      .max(MAX_N)
      .subtype(PROP_UNSIGNED)
      .description("等分數");
  b.add_input<decl::Vector>("Start")
      .default_value({0.0f, 0.0f, 0.0f})
      .subtype(PROP_TRANSLATION)
      .description("起始點(P0)");
  b.add_input<decl::Vector>("X Axis")
      .default_value({9.0f, 0.0f, 0.0f})
      .subtype(PROP_TRANSLATION)
      .description("變換後的 X 軸向量");
  b.add_input<decl::Vector>("Y Axis")
      .default_value({0.0f, 9.0f, 0.0f})
      .subtype(PROP_TRANSLATION)
      .description("變換後的 Y 軸向量");
  b.add_output<decl::Geometry>("卷殺折線");
}

static void node_layout(uiLayout *layout, bContext * /*C*/, PointerRNA *ptr)
{
  uiItemR(layout, ptr, "mode", UI_ITEM_R_EXPAND, nullptr, ICON_NONE);
}

//static void node_init(bNodeTree * /*tree*/, bNode *node){}

//static void node_update(bNodeTree *ntree, bNode *node){}

static Curves *create_entasis_line_curve(const int n, const float3 start, const float3 xAxis, const float3 yAxis)
{
  Curves *curves_id = bke::curves_new_nomain_single(n + 1, CURVE_TYPE_POLY);
  bke::CurvesGeometry &curves = curves_id->geometry.wrap();

  float sn = (n * (n + 1)) >> 1;

  float4x4 trs = float4x4(float4(xAxis), float4(yAxis), float4(math::cross(xAxis, yAxis)), float4(start));

  MutableSpan<float3> positions = curves.positions_for_write();
  for(const int k : IndexRange(n + 1))
  {
    float sk = (k * (k + 1)) >> 1;
    float tk = (k * (2 * n - k + 1)) >> 1;
    float3 pk = float3(sk, tk, 0) / sn;
    positions[k] = math::transform_point(trs, pk);
  }

  return curves_id;
}

static void node_geo_exec(GeoNodeExecParams params)
{
  Curves *curves = nullptr;
    curves = create_entasis_line_curve(
      params.extract_input<int>("N"),
      params.extract_input<float3>("Start"),
      params.extract_input<float3>("X Axis"),
      params.extract_input<float3>("Y Axis"));

  params.set_output("卷殺折線", GeometrySet::from_curves(curves));
}  

static void node_register()
{
  static blender::bke::bNodeType ntype;

  geo_node_type_base(&ntype, GEO_NODE_CURVE_PRIMITIVE_ENTASIS, "卷殺折線", NODE_CLASS_GEOMETRY);
  ntype.declare = node_declare;
  ntype.geometry_node_execute = node_geo_exec;
  ntype.draw_buttons = node_layout;
  blender::bke::nodeRegisterType(&ntype);
}
NOD_REGISTER_NODE(node_register)

}  // namespace blender::nodes::node_geo_curve_primitive_entasis_cc
