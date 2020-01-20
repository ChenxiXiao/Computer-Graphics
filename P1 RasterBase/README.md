Name: Cici Xiao

Information about the distance coloring point

If the current Z is close to the viewer, then write the pixel color as 
        Red: 255-abs(sin(10.0* zbuffer[y*g_width + x])) * 255;
        Blue:abs(cos(100.0* zbuffer[y*g_width + x])) * 255;
        Green:255-abs(sin(10.0* zbuffer[y*g_width + x])) * 255;


Citations for any downloaded code 
1. Dr. Philippe B. Laval. Mathematics for Computer Graphics - Barycentric Coordinates. From https://polylearn.calpoly.edu/AY_2019-2020/pluginfile.php/573699/mod_resource/content/6/barycentric.pdf
2. Syoyo Fujita. Obj Loader. From https://github.com/syoyo/tinyobjloader

