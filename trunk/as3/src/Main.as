package
{
	import cmodule.nes.CLibInit;
	
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.display.Sprite;
	import flash.display.StageAlign;
	import flash.display.StageScaleMode;
	import flash.events.Event;
	import flash.events.KeyboardEvent;
	import flash.net.URLLoader;
	import flash.net.URLLoaderDataFormat;
	import flash.net.URLRequest;
	import flash.utils.ByteArray;
	
	public class Main extends Sprite
	{
		
		private var urlLoader:URLLoader;
		private var bitmapData:BitmapData;
		private var bitmap:Bitmap;
		
		private var cLib:*;
		private var cMem:ByteArray;
		
		[Embed(source="vNes.nes",mimeType="application/octet-stream")]
		public var cClass:Class;
		
		public function Main()
		{
			super();
			setupStage();
			setupCModule();
			onLoaded();
		}
		
		private function onLoaded():void
		{
			var data = new cClass();
			cMem.position = cLib.initCartMem(data.length);
			cMem.writeBytes(data);
			cLib.noticeLoadCart();
			trace("start!~!!~~");
			start();
		}
		private function start():void
		{
			var cp:int = cLib.connectFCScreen();
			addEventListener(Event.ENTER_FRAME, function(event:Event):void
			{
				cLib.simulateFrame();
				
				cMem.position = cp; 
				bitmapData.setPixels(bitmapData.rect, cMem);
			});
		}
		
		
		private function setupCModule():void
		{
			var ns:Namespace = new Namespace("cmodule.nes");
			var cLibInit:CLibInit = new CLibInit();
			cLib = cLibInit.init();
			cMem = (ns::gstate).ds;
		}
		
		private function setupStage():void
		{
			stage.scaleMode = StageScaleMode.NO_SCALE;
			stage.align = StageAlign.TOP_LEFT;
			stage.frameRate = 20;
			bitmapData = new BitmapData(256, 240, false, 0);
			bitmap = new Bitmap(bitmapData);
			bitmap.width *= 2;
			bitmap.height *= 2;
			addChild(bitmap);
		}
	}
}