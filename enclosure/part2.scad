$fn=360;

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


difference(){
    union(){
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
    translate([44,44,-3])
    cylinder(r=2,h=45);
    
    translate([-44,44,-3])
    cylinder(r=2,h=45);

    translate([44,-44,-3])
    cylinder(r=2,h=45);

    translate([-44,-44,-3])
    cylinder(r=2,h=45);
}

// edge to raise for enclosure
difference(){
    union(){
        linear_extrude(height=10){
            mirror(v=[1,0,0])
            edge_primitive();
            edge_primitive();
            mirror(v=[0,1,0])
            mirror(v=[1,0,0])
            edge_primitive();
            mirror(v=[0,1,0])
            edge_primitive();
        }
        linear_extrude(height=10)
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

        // mounting holes for lid
        translate([44,44,0])
        difference(){
            cylinder(r=4.5,h=10);
            translate([0,0,-1])
            cylinder(r=2,h=45);
        }
        translate([-44,44,0])
        difference(){
            cylinder(r=4.5,h=10);
            translate([0,0,-1])
            cylinder(r=2,h=45);
        }

        translate([44,-44,0])
        difference(){
            cylinder(r=4.5,h=10);
            translate([0,0,-1])
            cylinder(r=2,h=45);
        }

        translate([-44,-44,0])
        difference(){
            cylinder(r=4.5,h=10);
            translate([0,0,-1])
            cylinder(r=2,h=45);
        }
    }
    // edge for warterproof connection
    translate([0,0,51])
    rotate(v=[0,1,0], a=180)
    union(){
        water_close_edge();
        mirror(v=[1,0,0])
        water_close_edge();
        mirror(v=[0,1,0])
        water_close_edge();
        mirror(v=[1,0,0])
        mirror(v=[0,1,0])
        water_close_edge();
    }
}
}
    translate([44,44,0])
    translate([0,0,-5])
    cylinder(r1=4.5,r2=2,h=10);

    translate([44,-44,0])
    translate([0,0,-5])
    cylinder(r1=4.5,r2=2,h=10);

    translate([-44,44,0])
    translate([0,0,-5])
    cylinder(r1=4.5,r2=2,h=10);

    translate([-44,-44,0])
    translate([0,0,-5])
    cylinder(r1=4.5,r2=2,h=10);
}


    
// mounting holes for lid



