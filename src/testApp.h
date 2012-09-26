#ifndef _TESTAPP
#define _TESTAPP

#include "ofMain.h"
#include "Graffiti.h"

class testApp : public ofBaseApp{
public:
	void        setup();
	void        update();
	void        draw();

	void        keyPressed  (int key);
	void        mousePressed(int x, int y, int button);
	void        mouseDragged(int x, int y, int button);
    void        mouseReleased();
	void        windowResized(int w, int h);
	void        dragEvent(ofDragInfo dragInfo);

	Graffiti    graffiti;
    ofImage     wall;
    float       hueCircle;
    bool        bWall, bColor,bEnableBackground;
};

#endif
