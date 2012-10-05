#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	ofEnableAlphaBlending();
    ofSetVerticalSync(true);
	
    graffiti.loadSettings();
    
    hueCircle   = 0.0;
    wall.loadImage("wall.jpg");
    
    bWall = true;
    bColor = true;
    
    graffiti.enableBackground(&wall);
    bEnableBackground = true;
}

//--------------------------------------------------------------
void testApp::update(){
	graffiti.update();
    
    if (bColor){
        hueCircle += 0.001;
        if (hueCircle >= 1.0){
            hueCircle = 0.0;
        }
        graffiti.brushColor.setHue(hueCircle*255);
    } else {
        graffiti.brushColor.set(0,255);
    }
    
    ofSetWindowTitle( ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void testApp::draw(){
    
    ofBackgroundGradient(ofColor::gray, ofColor::black);
   
    if (bWall){
        wall.draw(0, 0);  
    }
    
    ofEnableAlphaBlending();
    ofSetColor(255, 200);
	graffiti.draw();
}

//----------------------------------------------------------- EVENTOS
void testApp::keyPressed(int key){
	switch(key) {
		case 'f': ofToggleFullscreen(); break;
        case 'e': graffiti.bEditMode = !graffiti.bEditMode; break;
        case 'c': bColor = !bColor; break;
        case 'i': graffiti.init(graffiti.x,
                                graffiti.y,
                                graffiti.width,
                                graffiti.height); break;
        case ' ': graffiti.bClean = true; break;
        case '0': graffiti.nMode = 0; break;
        case '1': graffiti.nMode = 1; break;
        case '2': graffiti.nMode = 2; break;
        case '3': graffiti.nMode = 3; break;
        case 'w': bWall = !bWall; break;
        case OF_KEY_UP: graffiti.brushSize++; break;
        case OF_KEY_DOWN: graffiti.brushSize--; break;
	}
    
    if (key == 'b'){
        bEnableBackground = !bEnableBackground;
        if ( bEnableBackground )
            graffiti.enableBackground(&wall);
        else
            graffiti.disableBackground();
    }
    
    if (bColor)
        graffiti.brushColor.set(255,0,0);
}

void testApp::mousePressed(int x, int y, int button) {
}

void testApp::mouseDragged(int x, int y, int button){
}

void testApp::mouseReleased(){
    if (graffiti.bEditMode){
        graffiti.saveSettings();
    }
}

void testApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo info){

}
