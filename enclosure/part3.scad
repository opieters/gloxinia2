// this is the original enclosure

$fn=360;

module edge_primitive(){
    intersection(){
        difference(){
            translate([-46,34.5])
            translate([-1,1])
            circle(r=8);

            translate([-46,34.5])
            translate([-1,1])
            circle(r=6);
        }
        
        translate([-46,34.5])
        translate([-1,1])
        translate([8,-8])
        square([16,16],center=true);
    }

    translate([-48.5,27.5])
    square([1.5,2]);

    translate([-41,35.5])
    square([2,13]);

    translate([-40,46.5])
    square([80,2]);

    translate([-48.5,-28.5])
    square([2,57]);
}

module water_close_edge(){
    translate([0,47.5,40])
    rotate(v=[0,0,1],a=90)
    translate([0,40,0])
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=80)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    translate([0,0,40])
    translate([40,42,0])
    translate([0,5.5,0])
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=12)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    translate([40,47.5,40.75])
    cylinder(r1=1,r2=0.25,h=1.5,center=true);

    translate([0,0,40])
    translate([47,35.5,0])
    rotate(v=[0,0,1],a=180)
    rotate_extrude(angle=90)
    translate([1+6,0])
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    translate([0,0,40])
    translate([47,28.5,0])
    rotate(v=[0,0,1],a=90)
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=.5)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    translate([47.5,0,40])
    translate([0,28.5,0])
    rotate(v=[1,0,0],a=90)
    linear_extrude(height=57)
    polygon(points=[[-1,0], [1,0], [0.25,1.5], [-0.25, 1.5]]);

    translate([47.5,28.5,40.75])
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

// edge to raise for enclosure
linear_extrude(height=40){
    mirror(v=[1,0,0])
    edge_primitive();
    edge_primitive();
    mirror(v=[0,1,0])
    mirror(v=[1,0,0])
    edge_primitive();
    mirror(v=[0,1,0])
    edge_primitive();
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
square([93,97],center=true);
square([97,93],center=true);
translate([46.5,46.5])
circle(r=2);
translate([-46.5,46.5])
circle(r=2);
translate([46.5,-46.5])
circle(r=2);
translate([-46.5,-46.5])
circle(r=2);
}
// mounting holes of enclosure
translate([44,34,0])
cylinder(r=1.5,h=5,center=true);

translate([44,-34,0])
cylinder(r=1.5,h=5,center=true);

translate([-44,34,0])
cylinder(r=1.5,h=5,center=true);

translate([-44,-34,0])
cylinder(r=1.5,h=5,center=true);
}


// outer shell
linear_extrude(height=40)
difference() {
    union(){
        square([93,97],center=true);
        square([97,93],center=true);
        translate([46.5,46.5])
        circle(r=2);
        translate([-46.5,46.5])
        circle(r=2);
        translate([46.5,-46.5])
        circle(r=2);
        translate([-46.5,-46.5])
        circle(r=2);
    }
    square([91,95],center=true);
    square([95,91],center=true);
    translate([45.5,45.5])
    circle(r=2);
    translate([-45.5,45.5])
    circle(r=2);
    translate([45.5,-45.5])
    circle(r=2);
    translate([-45.5,-45.5])
    circle(r=2);
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

// standard pcb board mounting points
translate([-28,18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

translate([28,18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

translate([-28,-18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

translate([28,-18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

// moveable pcb board mounting points
translate([-41,18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

translate([41,18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

translate([-41,-18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

translate([41,-18])
difference(){
cylinder(h=6,r=4.5);
cylinder(h=7,r=2);
};

// mounting holes for lid
translate([44,44,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}
translate([-44,44,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}

translate([44,-44,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}

translate([-44,-44,0])
difference(){
    cylinder(r=4.5,h=40);
    translate([0,0,-1])
    cylinder(r=2,h=45);
}

// this is the PCB
color("green")
translate([0,0,10])
difference(){
    square([93,93],center=true);

    base_cutout();
    mirror(v=[0,1,0])
    base_cutout();
    mirror(v=[1,0,0])
    mirror(v=[0,1,0])
    base_cutout();
    mirror(v=[1,0,0])
    base_cutout();
    
    translate([-41,18])
        difference(){
        cylinder(h=6,r=4.5);
        cylinder(h=7,r=2);
        };

        translate([41,18])
        difference(){
        cylinder(h=6,r=4.5);
        cylinder(h=7,r=2);
        };

        translate([-41,-18])
        difference(){
        cylinder(h=6,r=4.5);
        cylinder(h=7,r=2);
        };

        translate([41,-18])
        difference(){
        cylinder(h=6,r=4.5);
        cylinder(h=7,r=2);
        };
    
}

