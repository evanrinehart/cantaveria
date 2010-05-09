
struct {
  int x;
  int y;
} camera;

int camera_x(){ return camera.x; }
int camera_y(){ return camera.y; }
void point_camera(int x, int y){
	camera.x = x;
	camera.y = y;
}
