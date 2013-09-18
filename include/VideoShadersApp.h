#include "cinder/ImageIo.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "dwmapi.h"
#include "ciUI.h"
#include "OscListener.h"
#include "GlslHotProg.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// RenderWindow class
class RenderWindow
{
	public:
		RenderWindow( string name, int width, int height, WindowRef wRef )
			: mName( name ), mWidth ( width ), mHeight ( height ), mWRef ( wRef )
		{
			
		}
		
		WindowRef mWRef;
		
	private:
		string mName;
		int mWidth;
		int mHeight;
		
};

class VideoShadersApp : public AppNative {
public:
	void prepareSettings( AppBasic::Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseMove( MouseEvent event );	
	void mouseDrag( MouseEvent event );	
	void update();
	void draw();
	void addFullScreenMovie( const fs::path &path );
	void fileDrop( FileDropEvent event );
	void shutdown();
	void keyDown( KeyEvent event );
		// windows mgmt
	void createNewWindow();
	void deleteWindows();
	vector<RenderWindow>		renderWindows;
	WindowRef					controlWindow;
	/*
		Shaders
	*/
	// PassThru
	void						shaderPassThru();
	// Light
	void						shaderLight();
	// Light
	void						shaderEdgeDetection();
	//ciUI
	ciUICanvas					*gui;   
	void guiEvent(ciUIEvent *event);
	ciUIRotarySlider			*beginRotary;
	ciUIRotarySlider			*endRotary;
	float						duration;
	float						b, e;

private:
	// windows and params
	int							mMainDisplayWidth;
	int							mRenderX;
	int							mRenderY;
	int							mRenderWidth;
	int							mRenderHeight;
	ci::params::InterfaceGl		mParams;

	gl::Texture					mTexture0;
	gl::Texture					mFrameTexture, mInfoTexture;

	Vec3i						iResolution;           // viewport resolution (in pixels)
	Vec3i						iChannelResolution;    
	float						iGlobalTime;           // shader playback time (in seconds)
	Vec3i						iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
	gl::GlslProg				mShader;
	int							iEmboss;
	qtime::MovieGl				mMovie;
	bool						movieLoaded;
	// Reymenta
	void						quitProgram();
	ColorAf						mBackgroundColor;
	ColorAf						mColor;
	osc::Listener 				receiver;
	// dynamic loading
	GlslHotProg					mHotShader;

};