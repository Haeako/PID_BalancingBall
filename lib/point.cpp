#include <point.h>
#include <communicate.h>
Point getPoint(Point &buffer)
  {
  sever host;
  host.createSocket();
  ho
    // Tách và chuyển đổi số đầu tiên thành int
    buffer.x= input.substring(0, spaceIndex).toFloat();
   if(!Serial.available())
   {
    //Serial.println("NOT available");
   }
    // Tách và chuyển đổi số thứ hai thành int
    buffer.y = input.substring(spaceIndex + 1, input.length()).toFloat();
  }
  return buffer;
  }
