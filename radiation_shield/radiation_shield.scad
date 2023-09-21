// all in mm
// thickness of the outer wall
d_wall = 1.5;

// outer radius of cone (incl. wall)
r_outer = 45;

// inner diameter of cone (incl. wall)
r_inner = 25;

r_opening = (r_inner+r_outer)/2;

// total height of the cap
h_cap = 25;

// distance between two caps (used for height)
cap_sep = 0.5*h_cap; // separation distance (as fraction of h_cap)

// total number of caps (incl. top and bottom cap, must be at least 2)
n_caps = 4;

// width of profile (tangent to inner circle)
p_w = 16;

// depth of profile (perpendicular to inner circle)
p_d = 5;

// width of opening in the profile (parallel to inner circle tangent)
p_o_w = 12;

// depth of opening in the profile
p_o_d = 3;

// width of slot in profile
p_o_s_w = 8;

// for more info on nut dimensions:
// https://en.wikipedia.org/wiki/Nut_(hardware)#Standard_metric_hex_nuts_sizes

// nut to attach internal and external shield
// height of the nut
nut_h = 3.2;

// inner radius of the nut
nut_r = 4/2;

// outer radius of the nut
nut_re = 8.1/2;

// margin used around to make sure nut fits
nut_margin = 0.1;

// nut to connect boards
// height of the nut
nut_bc_h = 2.4;

// inner radius of the nut
nut_bc_r = 3.0/2;

// outer radius of the nut
nut_bc_re = 6.4/2;

// margin used around to make sure nut fits
nut_bc_margin = 0.05;

// lock depth (height of upper part and)
lock_h = 2;
lock_d = 3;
lock_margin = 0.1;

p_margin = 0.5;
p_o_wm = p_o_w - p_margin;
p_o_dm = p_o_d - p_margin;

// for the additional support of beams in inner part
bs_margin = 3;
bs_margin_h = 3;

ps_margin = 5; //
ps_margin_h = 5;

// hook parameters

// radius of the two holes to mount
hook_opening_r = 2.5;

// length of one side of the hook (full length)
hook_length = 80;

// width of the hook
hook_width = 18;
// thickness
hook_thickness = 2;

// location of first hole, measured from the outer edge (away from right angle)
hole1_center_d = 16;

// location of second hole, measured from the outer edge (away from right angle)
hole2_center_d = 62.5;

// define modules
module cap(ri, ro, h_cap, d_wall) {
  difference() {
  cylinder(r1=ri+d_wall, r2=ro, h=h_cap);
  t = (ro - d_wall - ri) / h_cap;
  h = 5;
  r_i = ri - t * h;
  r_o = t * h + ro - d_wall;
  translate([0,0,-h]) cylinder(r1=r_i, r2=r_o, h=h_cap+2*h);
  }
}

module nut_support() {
  h = nut_h + 3*nut_h;
  t = h_cap / (r_outer - r_inner);
  dr = h / t;
  ri = r_outer - dr;
  dx = 2*(r_outer - r_opening); //(r_outer - r_inner);


  sc_points = [
    [0,0,0], [dx-dr, 0,0], [dx-dr, h, 0], [0, h, 0], 
    [0,0,h-nut_h], [dx-dr, 0,0], [dx-dr, h, ], [0, h, h-nut_h]];

  ps_faces = [
    [0, 1, 2, 3], // bottom
    [7, 6, 5, 4], // top
    [0, 4, 5, 1], // front
    [1, 5, 6, 2], // right
    [3, 7, 4, 0], // left
    [2, 6, 7, 3], // back
  ];

  points = [[0,0], [0,h-1.5*nut_h], [dx-dr, 0]];



  difference()
  {

    translate([-r_outer+dx,1.5*nut_re,0])
    difference()
    {
      intersection() {
        cylinder(r1=ri, r2=r_outer, h=h);
        //translate([0,0,-t*r_o + h_cap - d_wall]) cylinder(r1=0, r2= r_o, h=t*r_o);
        translate([0,-1.5*nut_re,-nut_h/2]) 
        cube([2*r_outer, 3*nut_re, 2*h]);
      }
      cube([2*(r_outer - dx), dx, 3*h], center=true);
      
      //color("red") translate([0,0,h_cap-2*d_wall]) cylinder(r1=r_inner-d_wall, r2=r_inner, h=8*d_wall);
    }

    translate(-[dx-dr,3.5*nut_re,h-3*d_wall]/2) 
    scale([2,2,2])
    translate([0, 3.5*nut_re, 0]) 
    rotate(a=90, v=[1,0,0]) 
    color("black") 
    linear_extrude(height=4*nut_re) 
    polygon(points=points);
    
    //translate([0,-2.5*nut_re/2,0]) color("white") polyhedron(points = sc_points, faces = ps_faces, connectivity = 3);
  }
}

module profile(h) {
  //translate([p_w/2,0,0])
  difference() {
    cube([p_w, p_d, h]);
    union() {
      translate([(p_w-p_o_w)/2,(p_d-p_o_d)/2,-0.25*h])
      cube([p_o_w, p_o_d, 1.5*h]);
      
      translate([(p_w-p_o_s_w)/2,p_d-1.5*(p_d-p_o_d)/2,-0.25*h]) 
      cube([p_o_s_w, 2*p_o_d, 1.5*h]);

    }
  }
}

module nut(h, rh, re, margin, fill=false) {
// h height of nut
// r radius of screw wire
// w width of nut between two horizontal sides

r = rh + margin;
ro = re + margin;
order = 6;
angles=[ for (i = [0:order-1]) i*(360/order) ];
   coords=[ for (th=angles) [ro*cos(th), ro*sin(th)] ];
   if(fill){
    linear_extrude(height=h) polygon(coords);
   } else {
    linear_extrude(height=h)
    difference(){
      polygon(coords);
      circle(r=r);
    }
   }
}

$fn=200;

// entire shape
union () {
  // flat top surface
  difference(){
      union(){
        cylinder(r1=r_inner, r2=r_inner+d_wall, h=d_wall);
        
        translate([0,7,d_wall])
        cylinder(r=13, h=d_wall);
      }

    //translate([0.3, 29.5, 0])
    cylinder(r=3, h=10*d_wall,center=true);
      
    translate([-7.25,7.2,d_wall])
    cube([14.5,4,2*d_wall]);
  }


  // draw each cap
  for( i = [0:n_caps-1]){
    color("red")
    translate([0,0,i*cap_sep]) cap(r_inner, r_outer, h_cap, d_wall);
  }

  // flat bottom surface 
  difference() 
  {
    h_slope = 7*d_wall;
    // draw filled shape
    {
      t = h_cap / (r_outer - r_inner - d_wall);
      r_o = r_outer;
      dr = (h_slope) / t;
      ri = r_outer - dr;
      color("red") translate([0,0,(n_caps-1)*cap_sep+h_cap-h_slope]) cylinder(r1=ri, r2=r_outer, h=h_slope);
    }
    // create skewed inner part
    color("blue") translate([0,0,(n_caps-1)*cap_sep+h_cap-h_slope+2*d_wall]) cylinder(r1=ri, r2=r_opening, h=h_slope, center=true);

    // hole in the centre
    translate([0,0,(n_caps-1)*cap_sep+h_cap-2*d_wall]) cylinder(r=r_opening, h=5*d_wall, center=true);
  }

  // create support for nuts to attach inner and outer parts
  translate([0,0,(n_caps-1)*cap_sep]) 
  difference(){
    // nut support
    for(i = [0:90:360-45]) {
      rotate(a=i, v=[0,0,1]) translate([r_inner, -1.5*nut_re, h_cap-nut_h-3*nut_h]) nut_support();
    }

    // nuts and holes for nuts
    union() {
      for(i = [0:90:360-45]) {
        rotate(a=i, v=[0,0,1]) 
        translate([r_inner+1.5*nut_re, 0, h_cap - nut_h]) 
        scale([1,1,2]) 
        nut(h=nut_h, rh=nut_r, re=nut_re, margin=-3*nut_margin, fill=true);
        
        rotate(a=i, v=[0,0,1]) 
        translate([r_inner+1.5*nut_re, 0, 10*nut_h]) 
        cylinder(r=nut_r + nut_margin, h=10*nut_h, center=true);
      }
    }
  }

  difference()
  {

    union(){

      difference(){
        cylinder(r=r_inner+d_wall, h=(n_caps-1)*cap_sep+h_cap);

        translate([0,0,-d_wall]) 
        cylinder(r=r_inner, h=(n_caps-1)*cap_sep+h_cap+2*d_wall);

        for(i = [0:90:360]){
          for(j = [1:n_caps]){
            rotate(a=i, v=[0,0,1]) 
            scale([1.5,1,1]) 
            translate([0,0,j*cap_sep-cap_sep/2])
            translate([0,((n_caps-1)*cap_sep+h_cap)/2,0]) 
            rotate(a=90, v=[1,0,0]) 
            cylinder(r=0.4*cap_sep, h=(n_caps-1)*cap_sep+h_cap);
          }
        }

        for(i = [45:90:360]){
          for(j = [1:n_caps]){
            rotate(a=i, v=[0,0,1]) 
            scale([1.5,1,1]) 
            translate([0,0,j*cap_sep-cap_sep/2])
            translate([0,((n_caps-1)*cap_sep+h_cap)/2,0]) 
            rotate(a=90, v=[1,0,0]) 
            cylinder(r=0.4*cap_sep, h=(n_caps-1)*cap_sep+h_cap);
          }
        }
        for(i = [22.5:45:360]) {
          j = n_caps + 1;
          rotate(a=i, v=[0,0,1]) 
          translate([0,0,j*cap_sep-cap_sep/2])
          translate([0,((n_caps-1)*cap_sep+h_cap)/2,0]) 
          rotate(a=90, v=[1,0,0]) 
          cylinder(r=0.3*cap_sep, h=(n_caps-1)*cap_sep+h_cap);
        }
        for(i = [45:90:360]) {
          j = n_caps + 1;
          rotate(a=i, v=[0,0,1]) 
          translate([0,0,j*cap_sep])
          translate([0,((n_caps-1)*cap_sep+h_cap)/2,0]) 
          rotate(a=90, v=[1,0,0]) 
          cylinder(r=0.3*cap_sep, h=(n_caps-1)*cap_sep+h_cap);
        }
      }
    }
  }
    }