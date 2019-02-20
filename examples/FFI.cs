// csc FFI.cs
// FFI.exe /path/to/file.osu
// make sure oppai.dll is in the same directory as FFI.exe
// see oppai.c for a full list of functions

using System;
using System.Runtime.InteropServices;

public class Program
{
  [DllImport(@"oppai.dll")]
  public static extern IntPtr ezpp_new();

  [DllImport(@"oppai.dll")]
  public static extern IntPtr ezpp(IntPtr ez, char[] map);

  [DllImport(@"oppai.dll")]
  public static extern float ezpp_pp(IntPtr ez);

  static void Main(string[] args)
  {
    IntPtr ez = ezpp_new();
    ezpp(ez, args[0].ToCharArray());
    Console.WriteLine($"{ezpp_pp(ez)} pp");
  }
}
