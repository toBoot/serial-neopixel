using System;
using System.IO.Ports;
using System.Collections.Concurrent;
using System.Threading;
using System.Windows.Media;
using System.Diagnostics;

namespace SerialLedCSharpLib
{
	public enum SerialError
	{
		None,
		PortError,
		UnexpectedError,
		NotConnected,
		Timeouted,
		TransmitionFailed,
		NothingToRead,
		NothingToWrite,
		HandshakeFailed
	}
	public class LedController
	{

		public LedController() 
		{
			rpi.BaudRate = 115200;
			rpi.DtrEnable = true;
			rpi.RtsEnable = true;
			rpi.ReadTimeout = timeout;
			rpi.WriteTimeout = timeout;
			rpi.Handshake = Handshake.None;

			//Damit nicht NULL!!
			rpiThread = new Thread(new ThreadStart(rpiThreadFunction));
		}
		~LedController()
		{
			if (IsConnected())
				Disconnect();
		}

		private struct returnValues
		{
			public returnValues(SerialError err, string answer)
			{
				Err = err;
				Answer = answer;
			}

			public SerialError Err;
			public string Answer;
		}

		private struct commandValues
		{
			public commandValues(string commandString, bool putOnErrorStack)
			{
				CommandString = commandString;
				PutOnErrorStack = putOnErrorStack;
			}

			public commandValues(string commandString)
			{
				CommandString = commandString;
				PutOnErrorStack = false;
			}

			public string CommandString;
			public bool PutOnErrorStack;
		}



		private SerialPort rpi = new SerialPort();
		private ConcurrentQueue <commandValues> commandQueue = new ConcurrentQueue<commandValues>();
		private ConcurrentQueue<returnValues> returnQueue = new ConcurrentQueue<returnValues>();
		private Thread rpiThread;

		private UInt16 version = 0;

		private const int timeout = 500;
		private const char front_delimiter = '>';
		private const char end_delimiter = '\n';





		//Wird vom Thread aufgerufen
		private void rpiThreadFunction()
		{
			//Beim starten des Threads wird errorStack gecleared
			while(returnQueue.Count > 0)
				returnQueue.TryDequeue(out _);

			if (commandQueue.IsEmpty)
			{
				returnQueue.Enqueue(new returnValues(SerialError.NothingToWrite, string.Empty));
				return;
			}


			if (!IsConnected())
			{
				returnQueue.Enqueue(new returnValues(SerialError.NotConnected, string.Empty));
				return;
			}

			commandValues cmd = new commandValues(string.Empty, false);

			
			while(!commandQueue.IsEmpty)
			{
				//Kann crashen, falls zwischen der if(!IsConnected()) Abfrage und dieser Zeile der RPI disconnected wird. Da Geschwindigkeit nicht essenziell wird, wurden hier Sicherheiten eingebaut.
				try
				{
					//Alles das noch auf dem Port ist lesen
					rpi.ReadExisting();


					if (commandQueue.TryDequeue(out cmd) && !string.IsNullOrEmpty(cmd.CommandString))
						rpi.Write(cmd.CommandString);
					else
					{
						returnQueue.Enqueue(new returnValues(SerialError.NothingToWrite, string.Empty));
						return;
					}

				}
				catch (System.InvalidOperationException e)
				{
					if(cmd.PutOnErrorStack)
						returnQueue.Enqueue(new returnValues(SerialError.NotConnected, string.Empty));
					return;
				}
				catch (TimeoutException e)
				{
					if(cmd.PutOnErrorStack)
						returnQueue.Enqueue(new returnValues(SerialError.Timeouted, string.Empty));
					return;
				}

				if (cmd.PutOnErrorStack)
				{
					returnValues rV;
					rV.Answer = readRpi(out rV.Err);
					returnQueue.Enqueue(rV);
				}

			}

		}





		//Wird nur einmal vom Thread aufgerufen!
		private string readRpi(out SerialError err)
		{
			if (!IsConnected())
			{
				err = SerialError.NotConnected;
				return string.Empty;
			}


			string serialBuffer = "";
			bool began = false;
			char inc_msg;

			Stopwatch stopwatch = new Stopwatch();
			
			//Kann crashen. Da Geschwindigkeit nicht essenziell wird, wurden hier Sicherheiten eingebaut.
			try
			{

				stopwatch.Start();
				while (stopwatch.Elapsed.TotalMilliseconds < timeout)
				{
					if (rpi.BytesToRead <= 0)
						continue;

					inc_msg = (char)rpi.ReadChar();

					if (inc_msg == front_delimiter)
					{
						serialBuffer = "";
						began = true;
					}

					if (began)
					{
						if (inc_msg == end_delimiter)
						{
							//Warten eines Zykluses des Raspberry
							Thread.Sleep(TimeSpan.FromMilliseconds(1));
							if (rpi.BytesToRead <= 0)
							{
								err = SerialError.None;
								return serialBuffer.Substring(0, serialBuffer.Length-1);
							}
							began = false;
							continue;
						}

						if (inc_msg != front_delimiter)
							serialBuffer += inc_msg;
					}

				}
			}
			catch(System.InvalidOperationException e)
			{
				err = SerialError.NotConnected;
				return string.Empty;
			}
			catch(TimeoutException e)
			{
				err = SerialError.Timeouted;
				return string.Empty;
			}


			if (!string.IsNullOrEmpty(serialBuffer))
			{
				err = SerialError.None;
				return serialBuffer.Substring(0, serialBuffer.Length - 1);
			}
			else
			{
				err = SerialError.TransmitionFailed;
				return string.Empty;
			}
		}


		private string stringCommand(string command, out SerialError err)
		{
			if (!IsConnected())
			{
				err = SerialError.NotConnected;
				return string.Empty;
			}

			if (string.IsNullOrEmpty(command))
			{
				err = SerialError.NothingToWrite;
				return string.Empty;
			}

			if (rpiThread.IsAlive)
				rpiThread.Join();

			commandQueue.Enqueue(new commandValues(command, true));
			rpiThread = new Thread(new ThreadStart(rpiThreadFunction));
			rpiThread.Start();
			rpiThread.Join();
			returnValues rV;

			if (!returnQueue.TryDequeue(out rV))
			{
				err = SerialError.UnexpectedError;
				return string.Empty;
			}

			err = rV.Err;
			return rV.Answer;
		}



		private bool boolCommand(string command, out SerialError err)
		{
			string answer = stringCommand(command, out err);

			if (err != SerialError.None)
				return false;


			if (answer == "true")
				return true;
			else if(answer == "false")
				return false;
			else
			{
				err = SerialError.TransmitionFailed;
				return false;
			}
		}





		private UInt16 intCommand(string command, out SerialError err)
		{
			string answer = stringCommand(command, out err);

			if(err != SerialError.None)
				return 0;

			if (UInt16.TryParse(answer, out UInt16 returnInt))
				return returnInt;
			else
			{
				err = SerialError.TransmitionFailed;
				return 0;
			}
		}



		
		private Color colorCommand(string command, out SerialError err)
		{
			string answer = stringCommand(command, out err);

			if (err != SerialError.None)
				return Colors.Black;

			byte r = 0, g = 0, b = 0;


			if(answer.Length == 11 && byte.TryParse(answer.Substring(0, 3), out r) && byte.TryParse(answer.Substring(4, 3), out g) && byte.TryParse(answer.Substring(8, 3), out b))
				return Color.FromRgb(r,g,b);

			err = SerialError.TransmitionFailed;
			return Colors.Black;
		}









		private void asyncCommand(string command)
		{
			if (!IsConnected() || string.IsNullOrEmpty(command))
				return;

			commandQueue.Enqueue(new commandValues(command));
			if (!rpiThread.IsAlive)
			{
				rpiThread = new Thread(new ThreadStart(rpiThreadFunction));
				rpiThread.Start();
			}
		}










		private SerialError handshake()
		{
			if (!IsConnected())
				return SerialError.NotConnected;

			SerialError err = SerialError.UnexpectedError;

			string answer = stringCommand(">GetVersion();", out err);

			if (err != SerialError.None)
				return err;

			if (string.IsNullOrEmpty(answer) || answer.Length < 9)
				return SerialError.HandshakeFailed;

			answer = answer.Substring(9, answer.Length - 9);

			if(!UInt16.TryParse(answer, out version))
			{
				version = 0;
				return SerialError.HandshakeFailed;
			}

			if (version > 0)
				return SerialError.None;
			else
				return SerialError.HandshakeFailed;
		}



		//Public Functions
		public bool Connect(string port, UInt16 ledCount, out SerialError err)
		{
			//Falls für Disconnect false zurück kommt muss auch err != SerialError.None sein!
			 if (IsConnected())
			{
				err = SerialError.None;
				return true; 
			}


			try
			{
				rpi.PortName = port;
				rpi.Open();

				rpi.ReadExisting();
			}
			catch(UnauthorizedAccessException e)
			{
				err = SerialError.PortError;
				Disconnect();
				return false;
			}
			catch(ArgumentOutOfRangeException e)
			{
				err = SerialError.PortError;
				Disconnect();
				return false;
			}
			catch(ArgumentException e)
			{
				err = SerialError.PortError;
				Disconnect();
				return false;
			}
			catch (System.IO.IOException e)
			{
				err = SerialError.PortError;
				Disconnect();
				return false;
			}
			catch (InvalidOperationException e)
			{
				err = SerialError.NotConnected;
				Disconnect();
				return false;
			}

			err = handshake();
			if (err != SerialError.None || !boolCommand(">SetLedCount(" + ledCount + ");", out err))
			{
				//Hier wird die Errornachricht von handshake() weitergeleitet, da sie wichtiger ist.
				Disconnect();
				return false;
			}
			else
				return true;
		}

		public bool Connect(string port, UInt16 ledCount)
		{
			return Connect(port, ledCount, out _); 
		}


		public bool AutoConnect(UInt16 ledCount)
		{
			string[] portList = SerialPort.GetPortNames();

			foreach (string port in portList)
			{
				if (Connect(port, ledCount))
					return true;
			}
			return false;
		}


		public bool Disconnect(out SerialError err)
		{
			if (IsConnected())
			{
				//Disconnected erst, wenn alle Aufträge abgearbeitet sind.
				if (rpiThread.IsAlive)
					rpiThread.Join();
				rpi.Close();
				err = SerialError.None;
				return true;
			}
			else
			{
				err = SerialError.NotConnected;
				return false;
			}
		}
		public bool Disconnect() 
		{ 
			return Disconnect(out _); 
		}




		public bool IsConnected() 
		{ 
			return rpi.IsOpen; ;
		}




		public UInt16 GetVersion() 
		{ 
			return version; 
		}





		//Public Controll Functions

		public bool Clear(out SerialError err)
		{
			return boolCommand(">Clear();", out err);
		}
		public bool Clear()	
		{
			return Clear(out _); 
		}
		public void ClearAsync()
		{
			asyncCommand(">Clear();");
		}





		public bool StopAllActions(out SerialError err)
		{
			return boolCommand(">StopAllActions();", out err);
		}
		public bool StopAllActions()
		{
			return StopAllActions(out _);
		}
		public void StopAllActionsAsync()
		{
			asyncCommand(">StopAllActions();");
		}




		public bool FinishAllActions(out SerialError err)
		{
			return boolCommand(">FinishAllActions();", out err);
		}
		public bool FinishAllActions()
		{
			return FinishAllActions(out _);
		}
		public void FinishAllActionsAsync()
		{
			asyncCommand(">FinishAllActions();");
		}




		public UInt16 GetLedCount(out SerialError err)
		{
			return intCommand(">GetLedCount();", out err);
		}
		public UInt16 GetLedCount()
		{
			return GetLedCount(out _);
		}




		public Color GetLedColor(UInt16 ledNumber, out SerialError err)
		{
			return colorCommand(">GetLedColor(" + ledNumber + ");", out err);
		}
		public Color GetLedColor(UInt16 ledNumber)
		{
			return GetLedColor(ledNumber, out _);
		}




		//SetStaticColor
		public bool SetStaticColor(byte red, byte green, byte blue, out SerialError err)
		{
			return boolCommand(">SetStaticColor("
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");", out err);
		}
		public bool SetStaticColor(byte red, byte green, byte blue)
		{
			return SetStaticColor(red, green, blue, out _);
		}
		public bool SetStaticColor(Color color, out SerialError err)
		{
			return SetStaticColor(color.R, color.G, color.B, out err);
		}
		public bool SetStaticColor(Color color)
		{
			return SetStaticColor(color.R, color.G, color.B);
		}
		public void SetStaticColorAsync(byte red, byte green, byte blue)
		{
			asyncCommand(">SetStaticColor("
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");");
		}
		public void SetStaticColorAsync(Color color)
		{
			SetStaticColorAsync(color.R, color.G, color.B);
		}




		public bool SetStaticColor(UInt16 ledNumber, byte red, byte green, byte blue, out SerialError err)
		{
			return boolCommand(">SetStaticColor("
				+ ledNumber
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");", out err);
		}
		public bool SetStaticColor(UInt16 ledNumber, byte red, byte green, byte blue)
		{
			return SetStaticColor(ledNumber, red, green, blue, out _);
		}
		public bool SetStaticColor(UInt16 ledNumber, Color color, out SerialError err)
		{
			return SetStaticColor(ledNumber, color.R, color.G, color.B, out err);
		}
		public bool SetStaticColor(UInt16 ledNumber, Color color)
		{
			return SetStaticColor(ledNumber, color.R, color.G, color.B);
		}
		public void SetStaticColorAsync(UInt16 ledNumber, byte red, byte green, byte blue)
		{
			asyncCommand(">SetStaticColor("
				+ ledNumber
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");");
		}
		public void SetStaticColorAsync(UInt16 ledNumber, Color color)
		{
			SetStaticColorAsync(ledNumber, color.R, color.G, color.B);
		}




		public bool SetStaticColor(UInt16 ledNumberFrom, UInt16 count, byte red, byte green, byte blue, out SerialError err)
		{
			return boolCommand(">SetStaticColor("
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");", out err);
		}

		public bool SetStaticColor(UInt16 ledNumberFrom, UInt16 count, byte red, byte green, byte blue)
		{
			return SetStaticColor(ledNumberFrom, count, red, green, blue, out _);
		}
		public bool SetStaticColor(UInt16 ledNumberFrom, UInt16 count, Color color, out SerialError err)
		{
			return SetStaticColor(ledNumberFrom, count, color.R, color.G, color.B, out err);
		}
		public bool SetStaticColor(UInt16 ledNumberFrom, UInt16 count, Color color)
		{
			return SetStaticColor(ledNumberFrom, count, color.R, color.G, color.B);
		}
		public void SetStaticColorAsync(UInt16 ledNumberFrom, UInt16 count, byte red, byte green, byte blue)
		{
			asyncCommand(">SetStaticColor("
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");");
		}
		public void SetStaticColorAsync(UInt16 ledNumberFrom, UInt16 count, Color color)
		{
			SetStaticColorAsync(ledNumberFrom, count, color.R, color.G, color.B);
		}



		public bool IsFading(out SerialError err)
		{
			return boolCommand(">IsFading();", out err);
		}

		public bool IsFading()
		{
			return IsFading(out _);
		}
		

		//FadeToStaticColor
		public bool FadeToStaticColor(UInt16 period, byte red, byte green, byte blue, out SerialError err)
		{
			return boolCommand(">FadeToStaticColor("
				+ period
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");", out err);
		}

		public bool FadeToStaticColor(UInt16 period, byte red, byte green, byte blue)
		{
			return FadeToStaticColor(period, red, green, blue, out _);
		}
		public bool FadeToStaticColor(UInt16 period, Color color, out SerialError err)
		{
			return FadeToStaticColor(period, color.R, color.G, color.B, out err);
		}
		public bool FadeToStaticColor(UInt16 period, Color color)
		{
			return FadeToStaticColor(period, color.R, color.G, color.B);
		}
		public void FadeToStaticColorAsync(UInt16 period, byte red, byte green, byte blue)
		{
			asyncCommand(">FadeToStaticColor("
				+ period
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");");
		}
		public void FadeToStaticColorAsync(UInt16 period, Color color)
		{
			FadeToStaticColorAsync(period, color.R, color.G, color.B);
		}



		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumber, byte red, byte green, byte blue, out SerialError err)
		{
			return boolCommand(">FadeToStaticColor("
				+ period
				+ ","
				+ ledNumber
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");", out err);
		}

		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumber, byte red, byte green, byte blue)
		{
			return FadeToStaticColor(period, ledNumber, red, green, blue, out _);
		}
		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumber, Color color, out SerialError err)
		{
			return FadeToStaticColor(period, ledNumber, color.R, color.G, color.B, out err);
		}
		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumber, Color color)
		{
			return FadeToStaticColor(period, ledNumber, color.R, color.G, color.B);
		}
		public void FadeToStaticColorAsync(UInt16 period, UInt16 ledNumber, byte red, byte green, byte blue)
		{
			asyncCommand(">FadeToStaticColor("
				+ period
				+ ","
				+ ledNumber
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");");
		}
		public void FadeToStaticColorAsync(UInt16 period, UInt16 ledNumber, Color color)
		{
			FadeToStaticColorAsync(period, ledNumber, color.R, color.G, color.B);
		}




		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumberFrom, UInt16 count, byte red, byte green, byte blue, out SerialError err)
		{
			return boolCommand(">FadeToStaticColor("
				+ period
				+ ","
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");", out err);
		}

		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumberFrom, UInt16 count, byte red, byte green, byte blue)
		{
			return FadeToStaticColor(period, ledNumberFrom, count, red, green, blue, out _);
		}
		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumberFrom, UInt16 count, Color color, out SerialError err)
		{
			return FadeToStaticColor(period, ledNumberFrom, count, color.R, color.G, color.B, out err);
		}
		public bool FadeToStaticColor(UInt16 period, UInt16 ledNumberFrom, UInt16 count, Color color)
		{
			return FadeToStaticColor(period, ledNumberFrom, count, color.R, color.G, color.B);
		}
		public void FadeToStaticColorAsync(UInt16 period, UInt16 ledNumberFrom, UInt16 count, byte red, byte green, byte blue)
		{
			asyncCommand(">FadeToStaticColor("
				+ period
				+ ","
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");");
		}
		public void FadeToStaticColorAsync(UInt16 period, UInt16 ledNumberFrom, UInt16 count, Color color)
		{
			FadeToStaticColorAsync(period, ledNumberFrom, count, color.R, color.G, color.B);
		}



		public bool StopFade(out SerialError err)
		{
			return boolCommand(">StopFade();", out err);
		}

		public bool StopFade()
		{
			return StopFade(out _);
		}
		public void StopFadeAsync()
		{
			asyncCommand(">StopFade();");
		}


		public bool SetGradient(byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd, out SerialError err)
		{
			return boolCommand(">SetGradient("
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");", out err);
		}

		public bool SetGradient(byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			return SetGradient(redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, out _);
		}
		public bool SetGradient(Color start, Color end, out SerialError err)
		{
			return SetGradient(start.R, start.G, start.B, end.R, end.G, end.B, out err);
		}
		public bool SetGradient(Color start, Color end)
		{
			return SetGradient(start.R, start.G, start.B, end.R, end.G, end.B);
		}
		public void SetGradientAsync(byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			asyncCommand(">SetGradient("
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");");
		}
		public void SetGradientAsync(Color start, Color end)
		{
			SetGradientAsync(start.R, start.G, start.B, end.R, end.G, end.B);
		}



		public bool SetGradient(UInt16 ledNumberFrom, UInt16 count, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd, out SerialError err)
		{
			return boolCommand(">SetGradient("
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");", out err);
		}

		public bool SetGradient(UInt16 ledNumberFrom, UInt16 count, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			return SetGradient(ledNumberFrom, count, redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, out _);
		}
		public bool SetGradient(UInt16 ledNumberFrom, UInt16 count, Color start, Color end, out SerialError err)
		{
			return SetGradient(ledNumberFrom, count, start.R, start.G, start.B, end.R, end.G, end.B, out err);
		}
		public bool SetGradient(UInt16 ledNumberFrom, UInt16 count, Color start, Color end)
		{
			return SetGradient(ledNumberFrom, count, start.R, start.G, start.B, end.R, end.G, end.B);
		}
		public void SetGradientAsync(UInt16 ledNumberFrom, UInt16 count, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			asyncCommand(">SetGradient("
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");");
		}
		public void SetGradientAsync(UInt16 ledNumberFrom, UInt16 count, Color start, Color end)
		{
			SetGradientAsync(ledNumberFrom, count, start.R, start.G, start.B, end.R, end.G, end.B);
		}




		public bool FadeToGradient(UInt16 period, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd, out SerialError err)
		{
			return boolCommand(">FadeToGradient("
				+ period
				+ ","
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");", out err);
		}
		public bool FadeToGradient(UInt16 period, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			return FadeToGradient(period, redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, out _);
		}
		public bool FadeToGradient(UInt16 period, Color start, Color end, out SerialError err)
		{
			return FadeToGradient(period, start.R, start.G, start.B, end.R, end.G, end.B, out err);
		}
		public bool FadeToGradient(UInt16 period, Color start, Color end)
		{
			return FadeToGradient(period, start.R, start.G, start.B, end.R, end.G, end.B);
		}
		public void FadeToGradientAsync(UInt16 period, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			asyncCommand(">FadeToGradient("
				+ period
				+ ","
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");");
		}
		public void FadeToGradientAsync(UInt16 period, Color start, Color end)
		{
			FadeToGradientAsync(period, start.R, start.G, start.B, end.R, end.G, end.B);
		}




		public bool FadeToGradient(UInt16 period, UInt16 ledNumberFrom, UInt16 count, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd, out SerialError err)
		{
			return boolCommand(">FadeToGradient("
				+ period
				+ ","
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");", out err);
		}

		public bool FadeToGradient(UInt16 period, UInt16 ledNumberFrom, UInt16 count, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			return FadeToGradient(period, ledNumberFrom, count, redStart, greenStart, blueStart, redEnd, greenEnd, blueEnd, out _);
		}
		public bool FadeToGradient(UInt16 period, UInt16 ledNumberFrom, UInt16 count, Color start, Color end, out SerialError err)
		{
			return FadeToGradient(period, ledNumberFrom, count, start.R, start.G, start.B, end.R, end.G, end.B, out err);
		}
		public bool FadeToGradient(UInt16 period, UInt16 ledNumberFrom, UInt16 count, Color start, Color end)
		{
			return FadeToGradient(period, ledNumberFrom, count, start.R, start.G, start.B, end.R, end.G, end.B);
		}
		public void FadeToGradientAsync(UInt16 period, UInt16 ledNumberFrom, UInt16 count, byte redStart, byte greenStart, byte blueStart, byte redEnd, byte greenEnd, byte blueEnd)
		{
			asyncCommand(">FadeToGradient("
				+ period
				+ ","
				+ ledNumberFrom
				+ ","
				+ count
				+ ","
				+ redStart
				+ ","
				+ greenStart
				+ ","
				+ blueStart
				+ ","
				+ redEnd
				+ ","
				+ greenEnd
				+ ","
				+ blueEnd
				+ ");");
		}
		public void FadeToGradientAsync(UInt16 period, UInt16 ledNumberFrom, UInt16 count, Color start, Color end)
		{
			FadeToGradientAsync(period, ledNumberFrom, count, start.R, start.G, start.B, end.R, end.G, end.B);
		}



		public bool IsPulsing(out SerialError err)
		{
			return boolCommand(">IsPulsing();", out err);
		}
		public bool IsPulsing()
		{
			return IsPulsing(out _);
		}

		public bool StopPulse(out SerialError err)
		{
			return boolCommand(">StopPulse();", out err);
		}
		public bool StopPulse()
		{
			return StopPulse(out _);
		}
		public void StopPulseAsync()
		{
			asyncCommand(">StopPulse();");
		}

		public bool FinishPulseUp(out SerialError err)
		{
			return boolCommand(">FinishPulseUp();", out err);
		}
		public bool FinishPulseUp()
		{
			return FinishPulseUp(out _);
		}
		public void FinishPulseUpAsync()
		{
			asyncCommand(">FinishPulseUp();");
		}

		public bool FinishPulseDown(out SerialError err)
		{
			return boolCommand(">FinishPulseDown();", out err);
		}
		public bool FinishPulseDown()
		{
			return FinishPulseUp(out _);
		}
		public void FinishPulseDownAsync()
		{
			asyncCommand(">FinishPulseDown();");
		}





		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, UInt16 pulseCount, bool finishPulseUp, byte red, byte green, byte blue, out SerialError err)
		{
			string finishPulseUpString;
			if (finishPulseUp)
				finishPulseUpString = "true";
			else
				finishPulseUpString = "false";

			return boolCommand(">StartPulse("
				+ periodUp
				+ ","
				+ periodHoldUp
				+ ","
				+ periodDown
				+ ","
				+ periodHoldDown
				+ ","
				+ pulseCount
				+ ","
				+ finishPulseUpString
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");", out err);
		}

		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, UInt16 pulseCount, bool finishPulseUp, byte red, byte green, byte blue)
		{
			return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, pulseCount, finishPulseUp, red, green, blue, out _);
		}

		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, UInt16 pulseCount, bool finishPulseUp, Color color, out SerialError err)
		{
			return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, pulseCount, finishPulseUp, color.R, color.G, color.B, out err);
		}

		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, UInt16 pulseCount, bool finishPulseUp, Color color)
		{
			return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, pulseCount, finishPulseUp, color.R, color.G, color.B, out _);
		}

		public void StartPulseAsync(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, UInt16 pulseCount, bool finishPulseUp, byte red, byte green, byte blue)
		{
			string finishPulseUpString;
			if (finishPulseUp)
				finishPulseUpString = "true";
			else
				finishPulseUpString = "false";

			asyncCommand("> StartPulse("
				+ periodUp
				+ ","
				+ periodHoldUp
				+ ","
				+ periodDown
				+ ","
				+ periodHoldDown
				+ ","
				+ pulseCount
				+ ","
				+ finishPulseUpString
				+ ","
				+ red
				+ ","
				+ green
				+ ","
				+ blue
				+ ");");
		}

		public void StartPulseAsync(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, UInt16 pulseCount, bool finishPulseUp, Color color)
		{
			StartPulseAsync(periodUp, periodHoldUp, periodDown, periodHoldDown, pulseCount, finishPulseUp, color.R, color.G, color.B);
		}





		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, byte red, byte green, byte blue, out SerialError err)
		{
			return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, 0, true, red, green, blue, out err);
		}

		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, byte red, byte green, byte blue)
		{
			return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, red, green, blue, out _);
		}

		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, Color color, out SerialError err)
		{
			return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, color.R, color.G, color.B, out err);
		}

		public bool StartPulse(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, Color color)
		{
			return StartPulse(periodUp, periodHoldUp, periodDown, periodHoldDown, color.R, color.G, color.B, out _);
		}

		public void StartPulseAsync(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, byte red, byte green, byte blue)
		{
			StartPulseAsync(periodUp, periodHoldUp, periodDown, periodHoldDown, 0, true, red, green, blue);
		}

		public void StartPulseAsync(UInt16 periodUp, UInt16 periodHoldUp, UInt16 periodDown, UInt16 periodHoldDown, Color color)
		{
			StartPulseAsync(periodDown, periodHoldUp, periodDown, periodHoldDown, color.R, color.G, color.B);
		}





		public bool IsCycling(out SerialError err)
		{
			return boolCommand(">IsCycling();", out err);
		}

		public bool IsCycling()
		{
			return IsCycling(out _);
		}


		public bool StartCycle(UInt16 period, bool direction, out SerialError err)
		{
			string w = ">StartCycle("
				+ period
				+ ",";

			if (direction)
				w += "true);";
			else
				w += "false);";

			return boolCommand(w, out err);
		}

		public bool StartCycle(UInt16 period, bool direction)
		{
			return StartCycle(period, direction, out _);
		}
		public void StartCycleAsync(UInt16 period, bool direction)
		{
			string w = ">StartCycle("
				+ period
				+ ",";

			if (direction)
				w += "true);";
			else
				w += "false);";

			asyncCommand(w);
		}



		public bool StopCycle(out SerialError err)
		{
			return boolCommand(">StopCycle();", out err);
		}
		public bool StopCycle()
		{
			return StopCycle(out _);
		}
		public void StopCycleAsync()
		{
			asyncCommand(">StopCycle();");
		}




		public bool FinishCycle(out SerialError err)
		{
			return boolCommand(">FinishCycle();", out err);
		}
		public bool FinishCycle()
		{
			return FinishCycle(out _);
		}
		public void FinishCycleAsync()
		{
			asyncCommand(">FinishCycle();");
		}
	}
}
