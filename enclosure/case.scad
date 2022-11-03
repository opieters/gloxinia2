$fn=360;

module edge_primitive(){
    translate([-50,38.5])
    intersection(){
        difference(){
            translate([-1,1])
            circle(r=8);

            translate([-1,1])
            circle(r=6);
        }
        
        translate([-1,1])
        translate([8,-8])
        square([16,16],center=true);
    }

    // small piece on the side
    translate([-52.5,29.5])
    square([1.5,2]);

    // straight peice to connect top
    color("red")
    translate([-45,39.5])
    square([2,13]);

    // top piece
    translate([-45,50.5])
    square([90,2]);

    // side piece
    translate([-51.5,0])
    translate([-1,-31.5])
    square([2,65]);
}

module water_close_edge(){
    // front
    translate([0,51.5,40])
    rotate(v=[0,0,1],a=90)
    translate([0,40,0])
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=84)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    // connector piece at the side
    translate([0,0,40])
    translate([44,46,0])
    translate([0,5.5,0])
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=12)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    // connector piece front and side
    translate([44,51.5,40.75])
    cylinder(r1=1,r2=0.25,h=1.5,center=true);

    // arc
    translate([0,0,40])
    translate([51,39.5,0])
    rotate(v=[0,0,1],a=180)
    rotate_extrude(angle=90)
    translate([1+6,0])
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    // small connector piece
    translate([0,0,40])
    translate([51,32.5,0])
    rotate(v=[0,0,1],a=90)
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=0.5)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    // side piece
    translate([51.5,0,40])
    translate([0,30.5,0])
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=63)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    // side piece connector
    translate([51.5,32.5,40.75])
    cylinder(r1=1,r2=0.25,h=1.5,center=true);
}

module base_cutout(){
    translate([-40-25,36.5])
    square([25,14]);

    translate([-46,34.5])
    translate([-2,2])
    circle(r=8);
}

/*translate([0,0,1.5])
union(){
    color("green")
    cube([76,93,3], center=true);
    color("green")
    cube([93,53,3], center=true);
}*/

//square([78,95],center=true);
//square([95,55],center=true);

// connector 
difference(){
    union(){
        // edge to raise for enclosure
        linear_extrude(height=40){
            edge_primitive();
            mirror(v=[1,0,0])
            edge_primitive();
            mirror(v=[0,1,0])
            mirror(v=[1,0,0])
            edge_primitive();
            mirror(v=[0,1,0])
            edge_primitive();
        }

        // outer shell
        linear_extrude(height=40){
        difference() {
            union(){
                square([101,105],center=true);
                square([105,101],center=true);
                translate([50.5,50.5])
                circle(r=2);
                translate([-50.5,50.5])
                circle(r=2);
                translate([50.5,-50.5])
                circle(r=2);
                translate([-50.5,-50.5])
                circle(r=2);
            }
            square([101,97],center=true);
            square([97,101],center=true);
            translate([48.5,48.5])
            circle(r=2);
            translate([-48.5,48.5])
            circle(r=2);
            translate([48.5,-48.5])
            circle(r=2);
            translate([-48.5,-48.5])
            circle(r=2);
        }
        }
    }
    
    union(){
    for(x=[-22.5, -7.5, 7.5, 22.5]){
        translate([x,53.5,7.6+1.28])
        rotate(a=90,v=[1,0,0])
        linear_extrude(height=6)
        minkowski()
        {
          circle(r=1.08);
          square([6.48,0.4],center=true);
        }
    }
    }
}

// edge for warterproof connection
water_close_edge();
mirror(v=[1,0,0])
water_close_edge();
mirror(v=[0,1,0])
water_close_edge();
mirror(v=[1,0,0])
mirror(v=[0,1,0])
water_close_edge();



// base plate
difference(){
linear_extrude(height=1){
square([105,101],center=true);
square([101,105],center=true);
translate([50.5,50.5])
circle(r=2);
translate([-50.5,50.5])
circle(r=2);
translate([50.5,-50.5])
circle(r=2);
translate([-50.5,-50.5])
circle(r=2);
}


// mounting holes of enclosure
translate([48,38,0])
cylinder(r=1.5,h=5,center=true);

translate([48,-38,0])
cylinder(r=1.5,h=5,center=true);

translate([-48,38,0])
cylinder(r=1.5,h=5,center=true);

translate([-48,-38,0])
cylinder(r=1.5,h=5,center=true);
}



// base plate
/*difference(){
    square([97,97],center=true);

    base_cutout();
    mirror(v=[0,1,0])
    base_cutout();
    mirror(v=[1,0,0])
    mirror(v=[0,1,0])
    base_cutout();
    mirror(v=[1,0,0])
    base_cutout();
}*/

// side pcb board mounting points
// moveable pcb board mounting points
for (a=[
      [32,21], [-32,21], [32,-21], [-32,-21],
      [41.5,21], [-41.5,21], [41.5,-21], [-41.5,-21],
      [33,42], [-33,42], [33,-42], [-33,-42],
      [0,-28], [0,28], [22,0], [-22,0],
]) {
translate(a)
difference(){
cylinder(h=6,r=4);
cylinder(h=7,r=2);
};
}


// mounting holes for lid
translate([48,48,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}
translate([-48,48,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}

translate([48,-48,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}

translate([-48,-48,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}

/*
for(x=[-22.5, -7.5, 7.5, 22.5]){
    color("blue")
    translate([x,53.5,7.6+1.28])
    rotate(a=90,v=[1,0,0])
    linear_extrude(height=6)
    minkowski()
    {
      circle(r=1.08);
      square([6.48,0.4],center=true);
    }
}
*/