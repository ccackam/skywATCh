import matplotlib.image as mpimg
import math

img=mpimg.imread('FAA_Flight_Plan_Form.png')

with open("/Users/kameroneves/Library/Application Support/Steam/steamapps/common/X-Plane 11/Resources/plugins/skywATCh/include/FAA_Flight_Plan_Form_cpp_rgb_array.h", "w") as f:
    first_time = True;
    f.write("bool flight_plan_form[699][1107] = {")
    for y in range(len(img)):
        first_time = True;
        for x in range(len(img[0])):
            r = img[y][x][0]
            g = img[y][x][1]
            b = img[y][x][2]
            if(math.sqrt(r**2+g**2+b**2)>0.9):
                if not first_time:
                    f.write(",")
                first_time = False;
                f.write("{")
                f.write('{},{}',x,y)
                f.write('}')
    f.write('};')
