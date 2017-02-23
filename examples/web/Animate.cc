//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "../../tools/Random.h"
#include "../../web/Animate.h"
#include "../../web/canvas_utils.h"
#include "../../web/emfunctions.h"
#include "../../web/web.h"

namespace UI = emp::web;

class MyAnimate : public UI::Animate {
private:
  UI::Document doc;
  UI::CanvasPolygon poly;
  UI::CanvasLine line;

  emp::Random random;

  size_t cx = 150;
  size_t cy = 150;
  size_t cr = 50;
  size_t can_size = 400;
  double poly_rot = 0.0;

public:
  MyAnimate() : doc("emp_base"), poly(200, 300, "red", "black"), line(5,5, 395,395, "green") {
    // How big should each canvas be?
    const size_t w = can_size;
    const size_t h = can_size;

    // Draw a simple circle animation on a canvas
    auto mycanvas = doc.AddCanvas(w, h, "can");
    mycanvas.Circle(cx, cy, cr, "blue", "purple");
    targets.push_back(mycanvas);

    // Draw the new polygon.
    poly.AddPoint(0,0).AddPoint(60,25).AddPoint(50,50).AddPoint(-50,50).AddPoint(25,40);
    mycanvas.Draw(poly);

    // Add a button.
    doc << "<br>";
    doc.AddButton([this](){
        ToggleActive();
        auto but = doc.Button("toggle");
        if (GetActive()) but.Label("Pause");
        else but.Label("Start");
      }, "Start", "toggle");

    doc << UI::Text("fps") << "FPS = " << UI::Live( [this](){return 1000.0 / GetStepTime();} ) ;
  }

  void DoFrame() {
    auto mycanvas = doc.Canvas("can");

    // Update the circle position.
    cx+=3;
    if (cx >= can_size + cr) cx -= can_size;

    // Draw the new circle.
    mycanvas.Clear();
    mycanvas.Circle(cx, cy, cr, "blue", "purple");
    if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "blue", "purple");

    // Update the polygon position
    poly_rot += 0.01;
    mycanvas.Rotate(poly_rot);
    mycanvas.Draw(poly);
    mycanvas.Rotate(-poly_rot);

    // Update the line.
    mycanvas.Draw(line);

    doc.Text("fps").Redraw();
  }
};

MyAnimate anim;

int main()
{
}
