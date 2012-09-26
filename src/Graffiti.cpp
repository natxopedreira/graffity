//
//  Graffity.cpp
//  TUIOpainter
//
//  Created by Patricio González Vivo on 6/9/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#include "Graffiti.h"

Graffiti::Graffiti(){
    
    //  Eventos de Mouse
    //
    ofAddListener(ofEvents().mouseDragged, this, &Graffiti::_mouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &Graffiti::_mouseReleased);
    
    //  Eventos del cliente TUIO 
    //
	ofAddListener(tuioClient.cursorAdded,this,&Graffiti::_tuioAdded);
	ofAddListener(tuioClient.cursorRemoved,this,&Graffiti::_tuioRemoved);
	ofAddListener(tuioClient.cursorUpdated,this,&Graffiti::_tuioUpdated);
	tuioClient.start(3333);
    
    //  En la primera vuelta limpia el FBO para que no halla basura y deja desactivado
    //  el modo de edición
    //
    bClean  = true; 
    bEditMode   = false;
    backgroundBaseDraw = NULL;
    eraseID     = -1;
    
    string stencilFragShader = "#version 120\n\
    #extension GL_ARB_texture_rectangle : enable\n\
    \n\
    uniform sampler2DRect imageTex;\n\
    uniform sampler2DRect stencilTex;\n\
    \n\
    void main (void){\n\
        vec2 st = gl_TexCoord[0].st;\n\
        vec4 image = texture2DRect(imageTex, st);\n\
        vec4 stencil = texture2DRect(stencilTex, st);\n\
        \n\
        float alpha = 1.0 - max(stencil.r,max(stencil.g,stencil.b));\n\
        gl_FragColor = vec4(image.rgb, min(alpha,image.a));\n\
    }\n";
    
    stencilShader.setupShaderFromSource(GL_FRAGMENT_SHADER, stencilFragShader);
    stencilShader.linkProgram();
}

void Graffiti::loadSettings(string _fileConfig){    
    ofxXmlSettings XML;
    
    if (XML.loadFile(_fileConfig)){
        
        oscReceiver.setup(XML.getValue("graffiti:OSCport", 3434));
        nMode   = XML.getValue("graffiti:defaultMode", 0);
        
        //  Graffiti Area
        //
        x = XML.getValue("graffiti:area:x", 0);
        y = XML.getValue("graffiti:area:y", 0);
        width = XML.getValue("graffiti:area:width", ofGetScreenWidth());
        height = XML.getValue("graffiti:area:height", ofGetScreenHeight());
        
        //  Brush properties
        //
        brushSize = XML.getValue("graffiti:brush:size", 8);
        bDroping = XML.getValue("graffiti:brush:droping", true);
        brushColor.r = XML.getValue("graffiti:brush:color:r", 255);
        brushColor.g = XML.getValue("graffiti:brush:color:g", 255);
        brushColor.b = XML.getValue("graffiti:brush:color:b", 255);
        brushColor.a = XML.getValue("graffiti:brush:color:a", 255);
        brush.loadImage(XML.getValue("graffiti:brush:image", "brush.png"));
        
        //  Stencil Mode
        //
        stencil.loadImage(XML.getValue("graffiti:stencilMode:image", "banksy.png"));
        stencilArea.x = XML.getValue("graffiti:stencilMode:area:x", ofGetScreenWidth()*0.5);
        stencilArea.y = XML.getValue("graffiti:stencilMode:area:y", 0);
        stencilArea.width = XML.getValue("graffiti:stencilMode:area:width", ofGetScreenWidth()*0.5);
        stencilArea.height = XML.getValue("graffiti:stencilMode:area:height", ofGetScreenHeight());
        
        //  Erase Mode
        //
        eraseArea.x = XML.getValue("graffiti:eraseMode:area:x", ofGetScreenWidth()*0.5);
        eraseArea.y = XML.getValue("graffiti:eraseMode:area:y", 0);
        eraseArea.width = XML.getValue("graffiti:eraseMode:area:width", ofGetScreenWidth()*0.5);
        eraseArea.height = XML.getValue("graffiti:eraseMode:area:height", ofGetScreenHeight());
        
        //  Poster Mode
        //
        poster.loadImage(XML.getValue("graffiti:posterMode:image", "poster.jpg"));
        posterArea.x = XML.getValue("graffiti:posterMode:area:x", ofGetScreenWidth()*0.5);
        posterArea.y = XML.getValue("graffiti:posterMode:area:y", 0);
        posterArea.width = XML.getValue("graffiti:posterMode:area:width", ofGetScreenWidth()*0.5);
        posterArea.height = XML.getValue("graffiti:posterMode:area:height", ofGetScreenHeight());
    }
    
    initGraffiti(x, y, width, height);
}

void Graffiti::saveSettings(string _fileConfig){
    ofxXmlSettings XML;
    
    if (XML.loadFile(_fileConfig)){
        
        if ( XML.pushTag("graffiti") ){ 
            
            XML.setValue("brush:size", brushSize);
            
            XML.setValue("area:x", x);
            XML.setValue("area:y", y);
            XML.setValue("area:width", width);
            XML.setValue("area:height", height);
            
            XML.setValue("stencilMode:area:x", stencilArea.x);
            XML.setValue("stencilMode:area:y", stencilArea.y);
            XML.setValue("stencilMode:area:width", stencilArea.width);
            XML.setValue("stencilMode:area:height", stencilArea.height);
            
            XML.setValue("eraseMode:area:x", eraseArea.x);
            XML.setValue("eraseMode:area:y", eraseArea.y);
            XML.setValue("eraseMode:area:width", eraseArea.width);
            XML.setValue("eraseMode:area:height", eraseArea.height);
            
            XML.setValue("posterMode:area:x", posterArea.x);
            XML.setValue("posterMode:area:y", posterArea.y);
            XML.setValue("posterMode:area:width", posterArea.width);
            XML.setValue("posterMode:area:height", posterArea.height);
            
            XML.popTag();
        }
        
        XML.saveFile();
    }
}

void Graffiti::initGraffiti(int _x, int _y, int _width, int _height){
    
    //  Setea las propiedades del rectangulo que contiene Graffiti
    //
    ofRectangle::set(_x, _y, _width, _height);
    
    //  Aloca el Fbo donde se va a dibujar
    //
	canvas.allocate(width, height, GL_RGBA);
    alphaMask.allocate(width, height, GL_RGBA);
    stencilBuffer.allocate(width, height, GL_RGBA);
    
    eraseID = -1;
    bClean  = true;
}

void Graffiti::enableBackground( ofBaseDraws *_baseDraw ){
    backgroundBaseDraw = _baseDraw;
    bClean = true;
}

void Graffiti::disableBackground(){
    backgroundBaseDraw = NULL;
    bClean = true;
}

//----------------------------------------------------------------------- UPDATING

void Graffiti::update(){
    tuioClient.getMessage();
    
    //  OSC receiver: para que otro pueda controlar esta app por OSC
    //
	while(oscReceiver.hasWaitingMessages()){
        
		//  Get the next message
        //
		ofxOscMessage m;
		oscReceiver.getNextMessage(&m);
        
        //  Check & do from OSC messages
        //
		if(m.getAddress() == "/graffiti/mode"){
			nMode = m.getArgAsFloat(0);
		} else if(m.getAddress() == "/graffiti/area"){
			x = m.getArgAsFloat(0);
            y = m.getArgAsFloat(1);
            width = m.getArgAsFloat(2);
            height = m.getArgAsFloat(3);
            initGraffiti(x,y,width,height);
		} else if(m.getAddress() == "/graffiti/brush/color"){
            brushColor = ofColor(m.getArgAsInt32(0),
                                 m.getArgAsInt32(1),
                                 m.getArgAsInt32(2),
                                 m.getArgAsInt32(3));
		} else if(m.getAddress() == "/graffiti/brush/radio"){
			brushSize = m.getArgAsFloat(0);
            
		} else if(m.getAddress() == "/graffiti/brush/dropping"){
            bDroping = !bDroping;
		} else if(m.getAddress() == "/graffiti/normalMode"){
            nMode = 0;
        } else if(m.getAddress() == "/graffiti/stencilMode"){
            nMode = 1;
        } else if(m.getAddress() == "/graffiti/stencilMode/area"){
            stencilArea.x = m.getArgAsFloat(0);
            stencilArea.y = m.getArgAsFloat(1);
            stencilArea.width = m.getArgAsFloat(2);
            stencilArea.height = m.getArgAsFloat(3);
		} else if(m.getAddress() == "/graffiti/eraseMode"){
            nMode = 2;
        } else if(m.getAddress() == "/graffiti/eraseMode/area"){
            eraseArea.x = m.getArgAsFloat(0);
            eraseArea.y = m.getArgAsFloat(1);
            eraseArea.width = m.getArgAsFloat(2);
            eraseArea.height = m.getArgAsFloat(3);
        } else if(m.getAddress() == "/graffiti/posterMode"){
            nMode = 3;
        } else if(m.getAddress() == "/graffiti/posterMode/area"){
            posterArea.x = m.getArgAsFloat(0);
            posterArea.y = m.getArgAsFloat(1);
            posterArea.width = m.getArgAsFloat(2);
            posterArea.height = m.getArgAsFloat(3);
        } else if(m.getAddress() == "/graffiti/clean"){
            clean();
		}
	}
    
    //  Check if need to clean de canvas
    //
    if (bClean){
        canvas.clear(0);
        canvas.begin();
        ofClear(0, 0);
        if (backgroundBaseDraw != NULL){
            ofSetColor(255, 255);
            backgroundBaseDraw->draw(-x, -y);
        }
        canvas.end();
        alphaMask.begin();
        ofClear(0, 0);
        alphaMask.end();
        paint.clear();
        bClean = false;
    }
    
    //  CANVAS ( donde pinta )
    //
    canvas.begin(0);
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        for (int i = 0; i < paint.size(); i++){
            paint[i]->update();
            paint[i]->draw();
        }
	canvas.end(0);
    canvas.setBlendMode( BLEND_NORMAL );
    canvas.update();

    
    if (nMode == 1){
        stencilModeUpdate();
    } else if (nMode == 2){
        eraseModeUpdate();
    } else if (nMode == 3){
        posterModeUpdate();
    }
}

//  STENCIL MODE 
//
void Graffiti::stencilModeUpdate(){
    
    //  Stencil Mask
    //
    stencilBuffer.src->begin();
    ofClear(0, 0);
    stencil.draw(stencilArea);
    stencilBuffer.src->end();
    
    stencilBuffer.dst->begin();
    ofClear(0,0);
    ofSetColor(255,255);
    
    stencilShader.begin();
    stencilShader.setUniformTexture("imageTex", canvas.getTextureReference() , 0);
    stencilShader.setUniformTexture("stencilTex", stencilBuffer.src->getTextureReference(), 1);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(width, 0); glVertex3f(width, 0, 0);
    glTexCoord2f(width, height); glVertex3f(width,height, 0);
    glTexCoord2f(0,height);  glVertex3f(0,height, 0);
    glEnd();
    
    stencilShader.end();
    stencilBuffer.dst->end();
}

//  ERASE MODE
//
void Graffiti::eraseModeUpdate(){
    //  Masking 
    //
    alphaMask.begin(0);
    ofClear(0,0);
    canvas.draw(0, 0);
    alphaMask.end(0);
    
    //  Content
    //
    alphaMask.setTexture(canvas.getTextureReference(), 1);
    
    alphaMask.update();
}

//  POSTER MODE
//
void Graffiti::posterModeUpdate(){
    
    alphaMask.begin(0); //  mask
    list<ofxTuioCursor*> cursors = tuioClient.getTuioCursors();
    
    //  Si hay sólo dos curseros TUIO
    //
    if (cursors.size() == 2){
        ofPoint p[2];
        
        //  Obtener las posiciones de los mismo
        //
        int i = 0;
        for (list<ofxTuioCursor*>::iterator it = cursors.begin(); it != cursors.end(); it++){
            p[i].set((*it)->getPosition().getX()*width, 
                     (*it)->getPosition().getY()*height);
            i++;
        }
        
        //  Calcula la rotacion, posición y tamaño
        //
        ofPoint diff = p[1] - p[0];
        float angle = -1*atan2(diff.x,diff.y)+(PI/2);
        float width = diff.length();
        float height = width*0.2;
        
        //  Dibujar el rodillo
        //
        ofPushMatrix();
        ofSetColor(255);
        ofTranslate(p[0]+diff*0.5);
        ofRotateZ( ofRadToDeg(angle) );
        ofRect(-width*0.5,-height*0.5,width,height);
        ofPopMatrix();
    }
    
    alphaMask.end(0);
    
    alphaMask.begin(1); //  content
    ofClear(0,0);
    ofColor(255,255);
    poster.draw(posterArea);
    alphaMask.end(1);
    
    alphaMask.update();
}

//-------------------------------------------------------------------- DRAWING

void Graffiti::draw(){
    
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(x, y);
    //  Dibuja lo pintado
    //
    if ( nMode == 0){
        canvas.draw(0,0);
    } else if (nMode == 1){
        stencilBuffer.dst->draw(0, 0);
    } else if (nMode == 2){
        alphaMask.draw(0,0);
    } else if (nMode == 3){
        alphaMask.draw(0,0);
    }
    
	//  Draggable area-boxes for Edit modes
    //
    if ( bEditMode ){
        
        //  Graffiti Area
        //
        ofNoFill();
        ofSetColor(245, 58, 135,200);
        ofRect(0,0,width,height);
        ofFill();
        ofSetColor(245, 58, 135,150);
        ofRect(-7,-7,14,14);
        ofRect(width-7,height-7,14,14);
        ofSetColor(245, 58, 135,255);
        ofDrawBitmapString("Graffiti Area",15,15);
        
        if ( nMode == 1){
            //  Stencil Position
            //
            ofNoFill();
            ofSetColor(255,150);
            stencil.draw(stencilArea);
            ofFill();
            ofSetColor(245, 58, 135,150);
            ofRect(stencilArea.x-7,stencilArea.y-7,14,14);
            ofRect(stencilArea.x+stencilArea.width-7,stencilArea.y+stencilArea.height-7,14,14);
            ofSetColor(245, 58, 135,255);
            ofDrawBitmapString("Stencil Area",stencilArea.x+15,stencilArea.y+15);
        } else if ( nMode == 2){
            //  Erase Area
            //
            ofNoFill();
            ofSetColor(245, 58, 135,200);
            ofRect(eraseArea);
            ofFill();
            ofSetColor(245, 58, 135,150);
            ofRect(eraseArea.x-7,eraseArea.y-7,14,14);
            ofRect(eraseArea.x+eraseArea.width-7,eraseArea.y+eraseArea.height-7,14,14);
            ofSetColor(245, 58, 135,255);
            ofDrawBitmapString("Erase Area",eraseArea.x+15,eraseArea.y+15);
        } else if ( nMode == 3){
            //  Poster Position
            //
            ofNoFill();
            ofSetColor(255,150);
            poster.draw(posterArea);
            ofFill();
            ofSetColor(245, 58, 135,150);
            ofRect(posterArea.x-7,posterArea.y-7,14,14);
            ofRect(posterArea.x+posterArea.width-7,posterArea.y+posterArea.height-7,14,14);
            ofSetColor(245, 58, 135,255);
            ofDrawBitmapString("Poster Area",posterArea.x+15,posterArea.y+15);
        }
    }
    
    ofPopMatrix();
    ofPopStyle();
}

void Graffiti::addPaint(ofPoint _pos,float _w, float _h, int _id) {
    ofPoint size    = ofPoint(abs( brushSize - _w),
                          abs( brushSize - _h),
                          0.0); 
    
    ofColor color  = brushColor;

    if ((nMode == 2) && (_id == eraseID )){
        //  Si este cursor TUIO se generó en la zona de borrado. Su ID va a estar registrado
        //  Desde aquí tan sólo debemos cambiar el color a uno con alpha 0
        //
        color.set(0,255);
    }
    
	float radio = (size.x + size.y)*0.5;
    int nLnk    = 0;
	int bDrop   = false;
    
    if (bDroping){
        
        //  Si esta "particula de pintura" esta muy cerca a otra 
        //  activa el dropeo de pintura... o sea... "chorrea pintura"
        //
        for (int i = paint.size()-1; i >= 0 ; i--){
            if (nLnk == 0)
                if ( paint[i]->getId() == _id ){
                    nLnk = i;
                    radio = ( paint[i]->getRadio() + radio ) * 0.5;
                    
                    if ( ( _pos.distance( paint[i]->getPosition() ) <= 2))
                        bDrop = true;
                }
        }
    }
    
	if ((_pos.x > 0) && (_pos.y > 0)){
        
        //  Cuando un blob esta fuera del área de trackeo de CCV suele generar un error por el que 
        //  envia ese cursor TUIO al 0,0. Para evitar errores es mejor preguntar siempre que no se
        //  encuentre en esa posición.
        //
		PaintParticle *p    = new PaintParticle( _id, _pos, color, radio);
        
		p->setAlpha( 255-ofMap( max(size.x,size.y) - min(size.x,size.y),0,max(size.x,size.y),0,255) );
        p->setBrush(&brush);
        
		if ( nLnk > 0 ) 
            p->setLnk(paint[nLnk]);
        
		if ( bDrop && (_id != eraseID )) 
            p->setDrops();
        
		paint.push_back(p);
	}
}

//----------------------------------------------------------- Mouse
void Graffiti::_mouseDragged(ofMouseEventArgs &e){
    ofPoint mouse = ofPoint(e.x,e.y);
    
    if (bEditMode){
        //  Drag coorners of graffitiArea
        //
        ofPoint A = ofPoint(x,y);
        
        ofPoint B = ofPoint(x+width,y+height);
        
        if ( A.distance(mouse) < 20 ){
            x += e.x - x;
            y += e.y - y;
        }
        
        if ( B.distance(mouse) < 20 ){
            width += e.x - x - width;
            height += e.y - y - height;
        }
        
        if ( nMode == 1){
            //  Drag coorners of stencil Position
            //
            ofPoint a = ofPoint(x+stencilArea.x,
                                y+stencilArea.y);
            
            ofPoint b = ofPoint(x+stencilArea.x+stencilArea.width,
                                y+stencilArea.y+stencilArea.height);
            
            if ( a.distance(mouse) < 20 ){
                stencilArea.x += e.x - x - stencilArea.x;
                stencilArea.y += e.y - y - stencilArea.y;
            }
            
            if ( b.distance(mouse) < 20 ){
                stencilArea.width += e.x - x - stencilArea.x - stencilArea.width;
                stencilArea.height += e.y - y - stencilArea.y - stencilArea.height;
            }
        } else if ( nMode == 2){
            //  Drag coorners of eraseArea
            //
            ofPoint a = ofPoint(x+eraseArea.x,
                                y+eraseArea.y);
            
            ofPoint b = ofPoint(x+eraseArea.x+eraseArea.width,
                                y+eraseArea.y+eraseArea.height);
            
            if ( a.distance(mouse) < 20 ){
                eraseArea.x += e.x - x - eraseArea.x;
                eraseArea.y += e.y - y - eraseArea.y;
            }
            
            if ( b.distance(mouse) < 20 ){
                eraseArea.width += e.x - x - eraseArea.x - eraseArea.width;
                eraseArea.height += e.y - y - eraseArea.y - eraseArea.height;
            }
        } else if ( nMode == 3){
            //  Drag coorners of poster Position
            //
            ofPoint a = ofPoint(x+posterArea.x,
                                y+posterArea.y);
            
            ofPoint b = ofPoint(x+posterArea.x+posterArea.width,
                                y+posterArea.y+posterArea.height);
            
            if ( a.distance(mouse) < 20 ){
                posterArea.x += e.x - x - posterArea.x;
                posterArea.y += e.y - y - posterArea.y;
            }
            
            if ( b.distance(mouse) < 20 ){
                posterArea.width += e.x - x - posterArea.x - posterArea.width;
                posterArea.height += e.y - y - posterArea.y - posterArea.height;
            }
        }
        
    }
    
}

void Graffiti::_mouseReleased(ofMouseEventArgs &e){
    if (bEditMode){
        initGraffiti(x, y, width, height);
    }
}

//----------------------------------------------------------- TUIO
void Graffiti::_tuioAdded(ofxTuioCursor & tuioCursor){
	ofPoint loc = ofPoint(tuioCursor.getX()*width,
                          tuioCursor.getY()*height,
                          0.0);
    
    if ( (nMode == 2) && ( eraseArea.inside(loc) ) ){
        
        //  Si el modo 2 esta activo y este cursor TUIO nace en el area
        //  de borrado. Entonces este ID queda registrado
        //
        eraseID = tuioCursor.getSessionId();
    }
    
    addPaint(loc,
             tuioCursor.getWidth()*width, 
             tuioCursor.getHeight()*height, 
             tuioCursor.getSessionId());
}

void Graffiti::_tuioUpdated(ofxTuioCursor &tuioCursor){
	ofPoint loc = ofPoint(tuioCursor.getX()*width,
                          tuioCursor.getY()*height,
                          0.0);
    
    addPaint(loc,
             tuioCursor.getWidth()*width, 
             tuioCursor.getHeight()*height, 
             tuioCursor.getSessionId());
    
	if (paint.size() > 50){
		paint.erase( paint.begin() );
	}
}

void Graffiti::_tuioRemoved(ofxTuioCursor & tuioCursor){
    
    //  Una vez que desaparezca el cursor TUIO que nacio en el area de borrado
    //  este registro desaparece
    //
    if ( eraseID == tuioCursor.getSessionId() ){
        eraseID = -1;
    }
}