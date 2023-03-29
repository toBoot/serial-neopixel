using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using SerialLedCSharpLib;

namespace TestCSharpApp
{
	class Program
	{
		static void Main(string[] args)
		{
			LedController ledController = new LedController();

			ledController.AutoConnect(14);

			System.Console.WriteLine("Version: " + ledController.GetVersion());




			System.Console.WriteLine(ledController.GetLedCount());

			System.Console.WriteLine(ledController.SetGradient(Color.FromRgb(100, 0, 0), Color.FromRgb(0,0,100)));
			ledController.StartCycle(5000, false);


			//System.Console.WriteLine(ledController.SetStaticColor(255, 255, 255));

			//for (int i = 0; i < 10; i++)
			//{
			//	for (int j = 0; j < 255; j++)
			//	{
			//		ledController.SetStaticColorAsync(Color.FromRgb((byte)j, (byte)(255 - j), 0));
			//	}
			//}

			//ledController.SetStaticColor(255, 123, 121);

			//System.Console.WriteLine(ledController.GetLedColor(1));

			//for (int i = 0; i < 10; i++)
			//{
			//	for (int j = 0; j < 255; j++)
			//	{
			//		ledController.SetStaticColor(Color.FromRgb((byte)j, (byte)(255 - j), 0));
			//	}
			//}


			//Console.WriteLine(ledController.GetLedCount());
		}
	}
}
