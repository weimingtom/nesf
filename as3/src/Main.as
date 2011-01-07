package
{
    import ands.fiano.IKeyboard;
    import ands.fiano.KeyManager;

    import cmodule.nes.CLibInit;

    import flash.display.Bitmap;
    import flash.display.BitmapData;
    import flash.display.Sprite;
    import flash.display.StageAlign;
    import flash.display.StageQuality;
    import flash.display.StageScaleMode;
    import flash.events.Event;
    import flash.events.KeyboardEvent;
    import flash.events.SampleDataEvent;
    import flash.events.TimerEvent;
    import flash.media.Sound;
    import flash.net.FileReference;
    import flash.net.SharedObject;
    import flash.net.URLLoader;
    import flash.ui.Keyboard;
    import flash.utils.ByteArray;
    import flash.utils.Timer;

    import net.hires.debug.Stats;

    [SWF(width="512" ,height="480", backgroundColor="0", fps="60")]
    public class Main extends Sprite implements IKeyboard
    {

        private var urlLoader:URLLoader;
        private var bitmapData:BitmapData;
        private var bitmap:Bitmap;

        private var cLibInit:CLibInit = new CLibInit();
        private var cLib:*;
        private var cMem:ByteArray;
        private var joyPad:int;
        private var joyPadOption:int;
        private var cp:int;
        private var keyBits:int = 0;
        private var keyBitsOption:int = 0;

        private var sound:Sound;
        private var buffer:ByteArray = new ByteArray;

        private var stateBytes:ByteArray;

		[Embed(source="/../games/双截龙3.nes", mimeType="application/octet-stream")]
        public var cClass:Class;
        public var so:SharedObject = SharedObject.getLocal("fcgame");
        public var initClass:Class;

        public function Main()
        {
            super();

            var owner:* = this;
            addEventListener(Event.ADDED_TO_STAGE, function(event:Event):void
            {
                removeEventListener(event.type, arguments.callee);
                setupStage();
                setupCModule();
                onLoaded();
                stage.frameRate = 60;
                stage.addChild(new Stats).visible = false;

                var key:KeyManager = new KeyManager();
                key.addKeyboard(owner);
                key.listen(stage);
            });
        }

        private function onKeyDown(event:KeyboardEvent):void
        {
            switch(event.keyCode)
            {
                case Keyboard.K: keyBits |= 0x1; break; //button A
                case Keyboard.J: keyBits |= 0x2; break; //button B
                case Keyboard.SPACE: keyBits |= 0x4; break; //select
                case Keyboard.ENTER: keyBits |= 0x8; break; //start

                case Keyboard.W: keyBits |= 0x10; break; //up
                case Keyboard.S: keyBits |= 0x20; break; //down
                case Keyboard.A: keyBits |= 0x40; break; //left
                case Keyboard.D: keyBits |= 0x80; break; //right

                case Keyboard.NUMPAD_3: keyBitsOption |= 0x1; break; //button A
                case Keyboard.NUMPAD_2: keyBitsOption |= 0x2; break; //button B

                case Keyboard.UP: keyBitsOption |= 0x10; break; //up
                case Keyboard.DOWN: keyBitsOption |= 0x20; break; //down
                case Keyboard.LEFT: keyBitsOption |= 0x40; break; //left
                case Keyboard.RIGHT: keyBitsOption |= 0x80; break; //right
            }
            cMem[joyPad] = (0xFF & keyBits);
            cMem[joyPadOption] = (0xFF & keyBitsOption);
        }

        private function onKeyUp(event:KeyboardEvent):void
        {
            switch(event.keyCode)
            {
                case Keyboard.K: keyBits &= 0xFE; break; //button A
                case Keyboard.J: keyBits &= 0xFD; break; //button B
                case Keyboard.SPACE: keyBits &= 0xFB; break; //select
                case Keyboard.ENTER: keyBits &= 0xF7; break; //select

                case Keyboard.W: keyBits &= 0xEF; break; //start
                case Keyboard.S: keyBits &= 0xDF; break;
                case Keyboard.A: keyBits &= 0xBF; break;
                case Keyboard.D: keyBits &= 0x7F; break;

                case Keyboard.NUMPAD_3: keyBitsOption &= 0xFE; break; //button A
                case Keyboard.NUMPAD_2: keyBitsOption &= 0xFD; break; //button B

                case Keyboard.UP: keyBitsOption &= 0xEF; break; //up
                case Keyboard.DOWN: keyBitsOption &= 0xDF; break; //down
                case Keyboard.LEFT: keyBitsOption &= 0xBF; break; //left
                case Keyboard.RIGHT: keyBitsOption &= 0x7F; break; //right

                case Keyboard.DELETE:
                    break;
            }
            cMem[joyPad] = (0xFF & keyBits);
            cMem[joyPadOption] = (0xFF & keyBitsOption);
        }

        public function actPress(keyCode:int, shift:Boolean):void
        {
            switch(keyCode)
            {
                case Keyboard.NUMBER_1:
                case Keyboard.NUMBER_2:
                case Keyboard.NUMBER_3:
                case Keyboard.NUMBER_4:
                    if(shift)
                    {
                        stateBytes.position = 0;
                        cLib.saveState(stateBytes);
                        so.data["s"+keyCode] = stateBytes
                        so.flush();
                    }
                    else
                    {
                        var bytes:ByteArray = so.data["s"+keyCode];
                        if(bytes != null)
                        {
                            stateBytes = bytes;
                            stateBytes.position = 0;
                            cLib.loadState(stateBytes);
                        }
                    }
                    break;
            }
        }

        public function actRelease(keyCode:int, shift:Boolean):void
        {
        }

        private function onLoaded():void
        {
            stateBytes = new ByteArray();

            cLibInit.supplyFile("rom", new cClass());
            cLib.initCartMem();
            joyPad = cLib.connectJoyPad();
            joyPadOption = joyPad + 1;
            cp = cLib.connectFCScreen();
            start();
        }

        private function start():void
        {
            var timer:Timer = new Timer(0);
            timer.addEventListener(TimerEvent.TIMER, update);
            timer.start();

            if(initClass != null)
            {
                cLib.loadState(new initClass);
            }
            addEventListener(Event.ENTER_FRAME, render);
        }


        private function render(event:Event):void
        {
            bitmapData.unlock();
            bitmapData.lock();
            cMem.position = cp;
            bitmapData.setPixels(bitmapData.rect, cMem);
        }

        private function update(event:TimerEvent):void
        {
            cLib.simulateFrame();
//            if(sound == null)
//            {
//                sound = new Sound();
//                sound.addEventListener(SampleDataEvent.SAMPLE_DATA, updateSound);
//                sound.play();
//            }
        }

        private function updateSound(event:SampleDataEvent):void
        {
            buffer.position = 0;
            cLib.playSound(buffer);
            buffer.position = 0;

            for(var i:int=0;i<buffer.length;i+=4)
            {
                event.data.writeFloat(buffer.readShort() /32768);
                event.data.writeFloat(buffer.readShort() /32768);
            }
        }


        private function setupCModule():void
        {
            var ns:Namespace = new Namespace("cmodule.nes");
            cLib = cLibInit.init();
            cMem = (ns::gstate).ds;
        }

        private function setupStage():void
        {
            stage.scaleMode = StageScaleMode.NO_SCALE;
            stage.align = StageAlign.TOP_LEFT;
            stage.quality = StageQuality.LOW;
            bitmapData = new BitmapData(256, 240, false, 0);
            bitmap = new Bitmap(bitmapData);
            bitmap.smoothing = true;
            bitmap.width *= 2;
            bitmap.height *= 2;
            stage.addEventListener(KeyboardEvent.KEY_DOWN, onKeyDown);
            stage.addEventListener(KeyboardEvent.KEY_UP ,onKeyUp);

            addChild(bitmap);
        }
    }
}
