import matplotlib.image as mpimg

img=mpimg.imread('FAA_Flight_Plan_Form.png')

with open("/Users/kameroneves/Library/Application Support/Steam/steamapps/common/X-Plane 11/Resources/plugins/skywATCh/include/FAA_Flight_Plan_Form_cpp_rgb_array.h", "w") as f:
    first_time = True;
    f.write("float flight_plan_form[699][1107][4] = {")
    for y in range(len(img)):
        if not first_time:
            f.write(",")
        f.write("{")
        first_time = True;
        for x in range(len(img[0])):
            if not first_time:
                f.write(",")
            f.write("{")
            first_time = False;
            r = img[y][x][0]
            g = img[y][x][1]
            b = img[y][x][2]
            a = 1.0
            f.write('{:.3f},{:.3f},{:.3f},{:.3f}'.format(r,g,b,a))
            f.write('}')
        f.write('}')
    f.write('};')
