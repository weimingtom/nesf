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
	import flash.ui.Keyboard;
	import flash.utils.ByteArray;
	
	public class Main extends Sprite
	{
		
		private var urlLoader:URLLoader;
		private var bitmapData:BitmapData;
		private var bitmap:Bitmap;
		
		private var cLib:*;
		private var cMem:ByteArray;
		private var joyPad:int;
		
		[Embed(source="vNes.nes",mimeType="application/octet-stream")]
		public var cClass:Class;
		private var keyBits:int = 0;
		
		public function Main()
		{
			super();
			setupStage();
			setupCModule();
			onLoaded();
		}
		
		private function onKeyDown(event:KeyboardEvent):void
		{
			cMem.position = joyPad;
			switch(event.keyCode)
			{
				case 90: keyBits |= 0x1; break;
				case 88: keyBits |= 0x2; break;
				case Keyboard.ENTER: keyBits |= 0x8; break;
				case Keyboard.UP: keyBits |= 0x10; break;
				case Keyboard.DOWN: keyBits |= 0x20; break;
				case Keyboard.LEFT: keyBits |= 0x40; break;
				case Keyboard.RIGHT: keyBits |= 0x80; break;
			}
			cMem.writeInt(keyBits);
		}
		
		private function onKeyUp(event:KeyboardEvent):void
		{
			cMem.position = joyPad;
			switch(event.keyCode)
			{
				case 90: keyBits &= 0xFE;; break;
				case 88: keyBits &= 0xFD; break;
				case Keyboard.ENTER: keyBits &= 0xF7; break;
				case Keyboard.UP: keyBits &= 0xEF; break;
				case Keyboard.DOWN: keyBits &= 0xDF; break;
				case Keyboard.LEFT: keyBits &= 0xBF; break;
				case Keyboard.RIGHT: keyBits &= 0x7F; break;
			}
			cMem.writeInt(keyBits);
		}
		
		private function onLoaded():void
		{
			var data:ByteArray = new cClass();
			cMem.position = cLib.initCartMem(data.length);
			cMem.writeBytes(data);
			joyPad = cLib.connectJoyPad();
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
			stage.addEventListener(KeyboardEvent.KEY_DOWN, onKeyDown);
			stage.addEventListener(KeyboardEvent.KEY_UP ,onKeyUp);
			
			addChild(bitmap);
		}
	}
}