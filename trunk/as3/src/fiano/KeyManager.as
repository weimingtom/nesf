package ands.fiano
{

    import flash.display.Stage;
    import flash.events.KeyboardEvent;
    import flash.ui.Keyboard;

    public class KeyManager
    {
        private var _isDownFunc:Function;
        private var downKeys:Vector.<int> = new Vector.<int>(256);
        private var keyboards:Vector.<IKeyboard> = new Vector.<IKeyboard>;

        private var isLocked:Boolean = false;

        public function addKeyboard(keyboard:IKeyboard):void
        {
            keyboards.push(keyboard);
        }


        public function lock():void
        {
            isLocked = true;
        }

        public function unlock():void
        {
            isLocked = false;
        }

        public function listen(stage:Stage):void
        {
            stage.addEventListener(KeyboardEvent.KEY_UP, function(event:KeyboardEvent):void
            {
                actRelease(event.keyCode, Boolean(downKeys[event.keyCode] & 0x100));
                downKeys[event.keyCode] = 0;
            })
			
			stage.addEventListener(KeyboardEvent.KEY_DOWN, function(event:KeyboardEvent):void
			{
				if(isDown(event.keyCode))
				{
					return;
				}
				actPress(event.keyCode, event.shiftKey);
				downKeys[event.keyCode] = 1;
			})
            _isDownFunc = function(keyCode:int):Boolean
            {
                return downKeys[keyCode] > 0;
            }
        }

        public function isDown(keyCode:int):Boolean
        {
            return _isDownFunc(keyCode);
        }

        public function actPress(keyCode:int, shift:Boolean):void
        {
            for each(var k:IKeyboard in keyboards)
            {
                k.actPress(keyCode, shift);
            }
        }

        public function actRelease(keyCode:int, shift:Boolean):void
        {
            for each(var k:IKeyboard in keyboards)
            {
                k.actRelease(keyCode, shift);
            }
        }
    }
}