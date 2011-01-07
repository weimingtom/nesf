package ands.fiano
{
    public interface IKeyboard
    {
        function actPress(keyCode:int, shift:Boolean):void;
        function actRelease(keyCode:int, shift:Boolean):void;
    }
}