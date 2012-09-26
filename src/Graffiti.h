//
//  Graffiti.h
//  TUIOpainter
//
//  Created by Patricio Gonzalez Vivo on 6/9/12.
//  Copyright (c) 2012 http://PatricioGonzalezVivo.com All rights reserved.
//

#ifndef TUIOpainter_Graffiti_h
#define TUIOpainter_Graffiti_h

#include "ofMain.h"

#include "PaintParticle.h"

#include "ofxFX.h"

#include "ofxOsc.h"

//  Implementación especial de ofxTuio que recibe el width y height de cada 
//  blob del CCV que deviene en puntero TUIO. Es importante habilitar el envío
//  del ancho y alto de los blobs en el CCV. 
//
#include "ofxTuio.h"

#include "ofxXmlSettings.h"

class Graffiti : public ofRectangle {
public:
    Graffiti();
    
    void    loadSettings(string _fileConfig = "config.xml");
    void    saveSettings(string _fileConfig = "config.xml");
    
    void    initGraffiti(int _x, int _y, int _width, int _height);
    void    enableBackground( ofBaseDraws *_baseDraw );
    void    disableBackground();
    
    void    clean(){ bClean    = true;};
    
    void    update();
    void    draw();
    
    ofColor brushColor;
    float   brushSize;
    
    int     nMode;      //  0 - Normal Graffiti Mode
                        //  1 - Stencils Mode
                        //  2 - Erase Mode 
                        //  3 - Poster Mode
    bool    bClean;
    bool    bDroping;
    bool    bEditMode;
    
private:
	void	_tuioAdded(ofxTuioCursor & tuioCursor);
	void	_tuioRemoved(ofxTuioCursor & tuioCursor);
	void	_tuioUpdated(ofxTuioCursor & tuioCursor);
    
    void    _mouseDragged(ofMouseEventArgs &e);
    void    _mouseReleased(ofMouseEventArgs &e);
    
    void	addPaint(ofPoint _pos, float _w, float _h, int _id);
    
    vector<PaintParticle*>  paint;
    
    myTuioClient    tuioClient;
    ofxOscReceiver  oscReceiver;
    ofBaseDraws     *backgroundBaseDraw;
    
    //  MODE 0 ( normal graffiti mode )
    //
    ofxBlend        canvas;
    ofImage         brush;
    
    //  MODE 1 ( Stencil Mode )
    //
    void            stencilModeUpdate();
    ofImage         stencil;
    ofRectangle     stencilArea;
    ofShader        stencilShader;
    ofxSwapBuffer   stencilBuffer;

    //  MODE 2 ( Erase Mode )
    void            eraseModeUpdate();
    ofRectangle     eraseArea;
    int             eraseID;
    
    //  MODE 3 ( Poster Mode )
    void            posterModeUpdate();
    ofxMask         alphaMask;
    ofImage         poster;
    ofRectangle     posterArea;
};

#endif
