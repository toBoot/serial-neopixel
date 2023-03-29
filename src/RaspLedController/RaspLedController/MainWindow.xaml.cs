using System;
using System.ComponentModel;
using System.Threading.Tasks;
using System.Windows;
using System.IO.Ports;
using System.Windows.Controls;
using System.Windows.Media;
using ColorPicker;
using MessageBox = System.Windows.MessageBox;

using SerialLedCSharpLib;
using System.Collections.ObjectModel;
using System.Windows.Threading;

namespace RaspLedController
{
	public partial class MainWindow : Window, INotifyPropertyChanged
	{
        public event PropertyChangedEventHandler PropertyChanged;

        public MainWindow()
        {
            InitializeComponent();

            timer.Interval = TimeSpan.FromSeconds(1);
            timer.Tick += TimerOnTick;
            timer.Start();

            PortList.Clear();
            PortList.Add("AUTO");

            foreach (var item in SerialPort.GetPortNames())
            {
                PortList.Add(item);
            }

            PortComboBox.ItemsSource = PortList;

			PortComboBox.SelectedIndex = 0;

			DataContext = this;



        }

        private LedController ledController = new LedController();

        private ObservableCollection<string> PortList = new ObservableCollection<string>();



        private DispatcherTimer timer = new DispatcherTimer();

        private void TimerOnTick(object sender, EventArgs e)
        {
            if (!ledController.IsConnected() && ConnectLabel.Content.ToString().Contains("Connected"))
            {
                ConnectLabel.Content = "Connection lost";
                ConnectLabel.Foreground = new SolidColorBrush(Colors.IndianRed);
                LedCountTextBox.IsEnabled = true;
            }
        }



        private ushort ledCount = 8;
        public ushort LedCount
        {
            get
            {
                return ledCount;
            }
            set
            {
                if (value != ledCount)
                {
                    ledCount = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("LedCount"));
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("LedsLeft"));
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("GradientLedsLeft"));
                }
            }
        }


        private ushort ledNumberFrom = 1;

        public ushort LedNumberFrom
        {
            get
            {
                return ledNumberFrom;
            }
            set
            {
                if (value != ledNumberFrom)
                {
                    ledNumberFrom = value;
                    if (count > LedsLeft)
                        Count = LedsLeft;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("LedNumberFrom"));
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("LedsLeft"));
                }
            }
        }


        public ushort LedsLeft
        {
            get
            {
                return (ushort)(ledCount - ledNumberFrom + 1);
            }
        }

        private ushort count = 1;
        public ushort Count
        {
            get
            {
                return count;
            }
            set
            {
                if (value != count)
                {
                    count = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("Count"));
                }
            }
        }



        private ushort fadePeriod = 1000;
        public ushort FadePeriod
        {
            get
            {
                return fadePeriod;
            }
            set
            {
                if (value != fadePeriod)
                {
                    fadePeriod = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("FadePeriod"));
                }
            }
        }

        private ushort cyclePeriod = 1000;
        public ushort CyclePeriod
        {
            get
            {
                return cyclePeriod;
            }
            set
            {
                if (value != cyclePeriod)
                {
                    cyclePeriod = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("CyclePeriod"));
                }
            }
        }



        private ushort gradientLedNumberFrom = 1;

        public ushort GradientLedNumberFrom
        {
            get
            {
                return gradientLedNumberFrom;
            }
            set
            {
                if (value != gradientLedNumberFrom)
                {
                    gradientLedNumberFrom = value;
                    if (gradientCount > GradientLedsLeft)
                        GradientCount = GradientLedsLeft;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("GradientLedNumberFrom"));
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("GradientLedsLeft"));
                }
            }
        }


        public ushort GradientLedsLeft
        {
            get
            {
                return (ushort)(ledCount - gradientLedNumberFrom + 1);
            }
        }

        private ushort gradientCount = 1;
        public ushort GradientCount
        {
            get
            {
                return gradientCount;
            }
            set
            {
                if (value != gradientCount)
                {
                    gradientCount = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("GradientCount"));
                }
            }
        }


        private ushort gradientFadePeriod = 1000;
        public ushort GradientFadePeriod
        {
            get
            {
                return gradientFadePeriod;
            }
            set
            {
                if (value != gradientFadePeriod)
                {
                    gradientFadePeriod = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("GradientCyclePeriod"));
                }
            }
        }


        private ushort ledInfoNumber = 1;
        public ushort LedInfoNumber
        {
            get
            {
                return ledInfoNumber;
            }
            set
            {
                if (value != ledInfoNumber)
                {
                    ledInfoNumber = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("LedInfoNumber"));
                }
            }
        }


        private byte infoRed = 0;
        public byte InfoRed
        {
            get
            {
                return infoRed;
            }
            set
            {
                if (value != infoRed)
                {
                    infoRed = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("InfoRed"));
                }
            }
        }

        private byte infoGreen = 0;
        public byte InfoGreen
        {
            get
            {
                return infoGreen;
            }
            set
            {
                if (value != infoGreen)
                {
                    infoGreen = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("InfoGreen"));
                }
            }
        }

        private byte infoBlue = 0;
        public byte InfoBlue
        {
            get
            {
                return infoBlue;
            }
            set
            {
                if (value != infoBlue)
                {
                    infoBlue = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("InfoBlue"));
                }
            }
        }


        private ushort pulsePeriodUp = 1500;
        public ushort PulsePeriodUp
        {
            get
            {
                return pulsePeriodUp;
            }
            set
            {
                if (value != pulsePeriodUp)
                {
                    pulsePeriodUp = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("PulsePeriodUp"));
                }
            }
        }
        

        private ushort pulsePeriodHoldUp = 1000;
        public ushort PulsePeriodHoldUp
        {
            get
            {
                return pulsePeriodHoldUp;
            }
            set
            {
                if (value != pulsePeriodHoldUp)
                {
                    pulsePeriodHoldUp = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("PulsePeriodHoldUp"));
                }
            }
        }
        
        private ushort pulsePeriodDown = 1500;
        public ushort PulsePeriodDown
        {
            get
            {
                return pulsePeriodDown;
            }
            set
            {
                if (value != pulsePeriodDown)
                {
                    pulsePeriodDown = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("PulsePeriodDown"));
                }
            }
        }
        
        private ushort pulsePeriodHoldDown = 1000;
        public ushort PulsePeriodHoldDown
        {
            get
            {
                return pulsePeriodHoldDown;
            }
            set
            {
                if (value != pulsePeriodHoldDown)
                {
                    pulsePeriodHoldDown = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("PulsePeriodHoldDown"));
                }
            }
        }
        
        private ushort pulseCount = 0;
        public ushort PulseCount
        {
            get
            {
                return pulseCount;
            }
            set
            {
                if (value != pulseCount)
                {
                    pulseCount = value;
                    PropertyChanged?.Invoke(this, new PropertyChangedEventArgs("PulseCount"));
                }
            }
        }

        private byte red = 255;
        private byte green = 0;
        private byte blue = 0;
        

        private byte gradientStartRed = 255;
        private byte gradientStartGreen = 0;
        private byte gradientStartBlue = 0;
        private byte gradientEndRed = 0;
        private byte gradientEndGreen = 0;
        private byte gradientEndBlue = 255;

        private byte pulseRed = 0;
        private byte pulseGreen = 0;
        private byte pulseBlue = 255;

        private string connectedPort = "AUTO";


        private void ContentButton_OnClick(object sender, RoutedEventArgs e)
        {
            if(PortComboBox.SelectedItem.ToString() == "AUTO")
			{
				if (ledController.AutoConnect(ledCount))
				{
                    ConnectLabel.Content = "Connected - Version: " + ledController.GetVersion();
                    ConnectLabel.Foreground = new SolidColorBrush(Colors.SeaGreen);

                    LedCount = ledController.GetLedCount();
                    LedCountTextBox.IsEnabled = false;
				}
				else
				{
                    MessageBox.Show("Es wurde kein Raspberry Pi Pico mit entsprechender Software gefunden.");
                }

            }

            else if (ledController.Connect(PortComboBox.SelectedItem.ToString(), ledCount))
			{
                ConnectLabel.Content = "Connected - Version: " + ledController.GetVersion();
                ConnectLabel.Foreground = new SolidColorBrush(Colors.SeaGreen);

                LedCount = ledController.GetLedCount();
                LedCountTextBox.IsEnabled = false;
            }
            else
            {
                MessageBox.Show("Verbindung zum Port " + PortComboBox.SelectedItem.ToString() + " konnte nicht aufgebaut werden.\n Das angegebene Board ist evtl. nicht kompatibel.");
            }
        }


        private void DisconnectButton_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.Disconnect();
            ConnectLabel.Content = "Disconnected";
            ConnectLabel.Foreground = new SolidColorBrush(Colors.IndianRed);
            LedCountTextBox.IsEnabled = true;
        }



        private void UpdateLedButton_OnClick(object sender, RoutedEventArgs e)
        {
            if (Fade.IsChecked == false)
                ledController.SetStaticColorAsync(LedNumberFrom, Count, red, green, blue);
            else if (Fade.IsChecked == true)
                ledController.FadeToStaticColorAsync(FadePeriod, LedNumberFrom, Count, red, green, blue);
        }

        private void SwitchLedOffButton_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.Clear();
            AutoUpdade.IsChecked = false;
            GradientAutoUpdate.IsChecked = false;
        }


        private void TextBox_TextToInt(object sender, TextChangedEventArgs e)
        {
            if (!(sender is TextBox))
                return;

            string text = (sender as TextBox).Text;
            if (UInt16.TryParse(text, out ushort output))
            {
                (sender as TextBox).Text = text;
            }
            else
            {
                (sender as TextBox).Text = string.Empty;
            }
        }

        private void StartCycle_OnClick(object sender, RoutedEventArgs e)
        {
            if (InverseDirection.IsChecked == true)
                ledController.StartCycle(CyclePeriod, false);
            else if (InverseDirection.IsChecked == false)
                ledController.StartCycle(CyclePeriod, true);
        }

        private void StopCycle_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.StopCycle();
        }



        private void UpdateGradient_OnClick(object sender, RoutedEventArgs e)
        {
            if (GradientFade.IsChecked == false)
                ledController.SetGradientAsync(GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);

            else if (GradientFade.IsChecked == true)
                ledController.FadeToGradientAsync(GradientFadePeriod, GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
        }

        private void StopAllActions_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.StopAllActions();
        }

        private void FinishAllActions_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.FinishAllActions();
        }

        private void GetLedInfo_OnClick(object sender, RoutedEventArgs e)
        {
            Color c = ledController.GetLedColor(LedInfoNumber);
            InfoRed = c.R;
            InfoGreen = c.G;
            InfoBlue = c.B;
        }

        private void FinishCycle_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.FinishCycle();
        }

        private void PortComboBox_OnDropDownOpened(object sender, EventArgs e)
        {
            string selected = PortComboBox.SelectedItem?.ToString();

            PortList.Clear();
            PortList.Add("AUTO");

            foreach (var item in SerialPort.GetPortNames())
            {
                if(!PortList.Contains(item))
                    PortList.Add(item);
            }

            for(int i = 0; i < PortList.Count; i++)
			{
                if (PortList[i] == selected)
                    PortComboBox.SelectedIndex = i;
			}

        }

        private void PortComboBox_OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
			if ((sender as ComboBox)?.SelectedItem == null)
				return;

            if (!ledController.IsConnected())
                return;

			if ((sender as ComboBox)?.SelectedValue.ToString() != connectedPort)
			{
                connectedPort = (sender as ComboBox)?.SelectedValue.ToString();
                ledController.Disconnect();
                ConnectLabel.Content = "Disconnected";
                ConnectLabel.Foreground = new SolidColorBrush(Colors.IndianRed);
                LedCountTextBox.IsEnabled = true;
            }
        }

        private void AutoUpdade_OnChecked(object sender, RoutedEventArgs e)
        {
            if (Fade.IsChecked == false)
                ledController.SetStaticColorAsync(LedNumberFrom, Count, red, green, blue);
            else if (Fade.IsChecked == true)
                ledController.FadeToStaticColorAsync(FadePeriod, LedNumberFrom, Count, red, green, blue);
        }
		private void AutoUpdateGradient_OnChecked(object sender, RoutedEventArgs e)
		{
            if (GradientFade.IsChecked == false)
                ledController.SetGradientAsync(GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);

            else if (GradientFade.IsChecked == true)
                ledController.FadeToGradientAsync(GradientFadePeriod, GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
        }

        private void GeneralSlider_OnValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (!ledController.IsConnected())
                return;

            if (AutoUpdade.IsChecked == true && Fade.IsChecked == false)
                ledController.SetStaticColorAsync(LedNumberFrom, Count, red, green, blue);
            else if (AutoUpdade.IsChecked == true && Fade.IsChecked == true)
                ledController.FadeToStaticColorAsync(FadePeriod, LedNumberFrom, Count, red, green, blue);
        }

        private void GradientSlider_OnValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (!ledController.IsConnected())
                return;

            if (GradientAutoUpdate.IsChecked == true && GradientFade.IsChecked == false)
                ledController.SetGradientAsync(GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
            else if (GradientAutoUpdate.IsChecked == true && GradientFade.IsChecked == true)
                ledController.FadeToGradientAsync(GradientFadePeriod, GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
        }

        private void GeneralColorPicker_ColorChanged(object sender, RoutedEventArgs e)
		{
            if (!(sender is ColorPicker.PortableColorPicker) || GradientAutoUpdate is null || GradientFade is null)
                return;

            red = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_R;
            green = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_G;
            blue = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_B;

			if (AutoUpdade.IsChecked == true && Fade.IsChecked == false)
				ledController.SetStaticColorAsync(LedNumberFrom, Count, red, green, blue);
			else if (AutoUpdade.IsChecked == true && Fade.IsChecked == true)
				ledController.FadeToStaticColorAsync(FadePeriod, LedNumberFrom, Count, red, green, blue);
		}

		private void GradientFromColor_ColorChanged(object sender, RoutedEventArgs e)
		{
            if (!(sender is ColorPicker.PortableColorPicker) || GradientAutoUpdate is null || GradientFade is null)
                return;

            gradientStartRed = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_R;
            gradientStartGreen = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_G;
            gradientStartBlue = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_B;

            if (GradientAutoUpdate.IsChecked == true && GradientFade.IsChecked == false)
                ledController.SetGradientAsync(LedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
            else if (GradientAutoUpdate.IsChecked == true && GradientFade.IsChecked == true)
                ledController.FadeToGradientAsync(GradientFadePeriod, GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
        }

		private void GradientToColor_ColorChanged(object sender, RoutedEventArgs e)
		{
            if (!(sender is ColorPicker.PortableColorPicker) || GradientAutoUpdate is null || GradientFade is null)
                return;

            gradientEndRed = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_R;
            gradientEndGreen = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_G;
            gradientEndBlue = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_B;

			if (GradientAutoUpdate.IsChecked == true && GradientFade.IsChecked == false)
				ledController.SetGradientAsync(LedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
			else if (GradientAutoUpdate.IsChecked == true && GradientFade.IsChecked == true)
				ledController.FadeToGradientAsync(GradientFadePeriod, GradientLedNumberFrom, GradientCount, gradientStartRed, gradientStartGreen, gradientStartBlue, gradientEndRed, gradientEndGreen, gradientEndBlue);
		}
        
        private void PulseColorPicker_ColorChanged(object sender, RoutedEventArgs e)
        {
            if (!(sender is ColorPicker.PortableColorPicker) || GradientAutoUpdate is null || GradientFade is null)
                return;

            pulseRed = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_R;
            pulseGreen = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_G;
            pulseBlue = (byte)(sender as ColorPicker.PortableColorPicker).Color.RGB_B;
        }

        private void StartPulse_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.StartPulseAsync(pulsePeriodUp, pulsePeriodHoldUp, pulsePeriodDown, pulsePeriodHoldDown, pulseCount, finishFadeUpCheckBox.IsChecked.Value, pulseRed, pulseGreen, pulseBlue);
        }

        private void StopPulse_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.StopPulseAsync();
        }

        private void FinishPulseUp_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.FinishPulseUpAsync();
        }

        private void FinishPulseDown_OnClick(object sender, RoutedEventArgs e)
        {
            ledController.FinishPulseDownAsync();
        }
    }
}
