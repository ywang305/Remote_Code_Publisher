/////////////////////////////////////////////////////////////////////
// MainWindow.cs -  WPF GUI                                        //
//                                                                 //
// Yaodong Wang CSE687 - OOD, Spring 2017                          //
/////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * this GUI demo to simulate two individual clients that let them run
 *     on seperated threads;  .
 * 
 * Required Files:
 * - Shim dll
 * 
 * Maintenance History:
 * --------------------
 * ver 1.0 : 26 April 2017
 * - first release
 */

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.Reflection;

namespace WPFgui
{
    public class Client
    {
        public String clientId { set; get; }
        public int myPort { get; set; }
        public int toPort { get; set; }
        public Shim shim { get; set; }
        public Thread myThread { get; set; }
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private Client Jim, Mike; // for testing, in tab 2 "Network Configuration"
        private Client Fawcett, Jackson;

        public MainWindow()
        {
            Console.Title = "WPFgui Clients Runing on Threads";
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            InitializeFawcett();
            InitializeJackson();
            DemoFawcett();
            DemoJackson();
        }

        #region autmatic demo send cpp/h src files and request list

        private void DemoFawcett()
        {
            String srcCodeDirPath = getFullPathByCheckingParent("Repository-Client-Project-3");
            Directory.SetCurrentDirectory(srcCodeDirPath);
            String fullClientWannaSendFilesPath = getFullPathByCheckingSubDir(Fawcett.clientId);
            if ( fullClientWannaSendFilesPath != null)
            {
                Fawcett.shim.updateMyClientDir(fullClientWannaSendFilesPath);
                foreach (var fileFullPath in Directory.GetFiles(fullClientWannaSendFilesPath))
                {
                    var filename = System.IO.Path.GetFileName(fileFullPath);
                    Fawcett.shim.handDownPostFileMessage(filename, "localhost:" + Fawcett.toPort.ToString());
                }
                /// must send a quit to tell code publisher create database!
                Fawcett.shim.handDownPostMessage("quit", "localhost:" + Fawcett.toPort.ToString());
            }
            // get all list
            Fawcett.shim.handDownGetFileMessage("listall", "localhost:" + Fawcett.toPort.ToString());

            Fawcett_Info_TextBlock.Text = "running Fawcett's demo, please wait seconds until server returns results";
        }

        private void DemoJackson()
        {
            String srcCodeDirPath = getFullPathByCheckingParent("Repository-Client-Project-3");
            Directory.SetCurrentDirectory(srcCodeDirPath);
            String fullClientWannaSendFilesPath = getFullPathByCheckingSubDir(Jackson.clientId);
            if (fullClientWannaSendFilesPath != null)
            {
                Jackson.shim.updateMyClientDir(fullClientWannaSendFilesPath);
                foreach (var fileFullPath in Directory.GetFiles(fullClientWannaSendFilesPath))
                {
                    var filename = System.IO.Path.GetFileName(fileFullPath);
                    Jackson.shim.handDownPostFileMessage(filename, "localhost:" + Jackson.toPort.ToString());
                }

                /// must send a quit to tell code publisher create database!
                Jackson.shim.handDownPostMessage("quit", "localhost:" + Jackson.toPort.ToString());
            }
            // get all list
            Jackson.shim.handDownGetFileMessage("listall", "localhost:" + Jackson.toPort.ToString());

            Jackson_Info_TextBlock.Text = "running Jackson's demo, please wait seconds until server returns results";
        }

        #endregion



        #region "Tab File Xmitter - Client Fawcett"

        /// <summary>
        /// upload file to server
        /// </summary>
        private void Fawcett_Btn_Upload_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog dlg = new System.Windows.Forms.OpenFileDialog();

            String srcCodeDirPath = getFullPathByCheckingParent("Repository-Client-Project-3");

            dlg.InitialDirectory = srcCodeDirPath;
            //dlg.Filter = "cpp|*.cpp|header|*.h|c#|*cs";
            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                String filename = dlg.SafeFileName;
                Fawcett_Info_TextBlock.Text = dlg.SafeFileName;
                Fawcett.shim.handDownPostFileMessage(filename, "localhost:" + Fawcett.toPort.ToString());
                
                /// must send a quit to tell code publisher create database!
                //Fawcett.shim.handDownPostMessage("quit", "localhost:" + Fawcett.toPort.ToString());
            }
        }
        /// <summary>
        /// get all category 
        /// </summary>
        private void Fawcett_Btn_GetCateg_Click(object sender, RoutedEventArgs e)
        {
            Fawcett_Info_TextBlock.Text = "sent a category request message to server";

            Fawcett.shim.handDownGetFileMessage("listall", "localhost:" + Fawcett.toPort.ToString());      
        }

        /// <summary>
        ///   browsing htms
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>

        private void Fawcett_Btn_Blowser_Click(object sender, RoutedEventArgs e)
        {

            string rcvDirPath = getFullPathByCheckingParent("Repository-Server-Project-3");
            
            foreach( var filename in Fawcett_DownloadList.Items)
            {
                string brws = rcvDirPath + @"\" + filename + ".htm";
                System.Diagnostics.Process.Start(brws);
            }
        }


        /// <summary>
        ///  category item selected
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Fawcett_CategoryList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox sdr = (ListBox)sender;
            if (sdr.Items.Count == 0) return;
            var endpoint = sdr.SelectedValue;
            Fawcett.shim.handDownGetFileMessage( endpoint.ToString(), "localhost:" + Fawcett.toPort.ToString());

            Fawcett_Info_TextBlock.Text = "category: " + endpoint + " selected,  list its all files";
            Fawcett_FileList.Items.Clear();
        }

        /// <summary>
        ///  file list item selected
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Fawcett_FileList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (((ListBox)sender).Items.Count == 0) return;

            String filename = (String)((ListBox)sender).SelectedValue;

            Fawcett_Info_TextBlock.Text = "download file " + filename + " and its dependency.   You can browze webpage by click button 'Start Broswer'" ;
            Fawcett.shim.handDownGetFileMessage(filename, "localhost:" + Fawcett.toPort.ToString());

            Fawcett_DownloadList.Items.Clear();
        }

        #endregion

        #region Tab File Xmitter - Client Jackson

        /// <summary>
        ///  upload a file to server
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Jackson_Btn_Upload_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog dlg = new System.Windows.Forms.OpenFileDialog();

            String srcCodeDirPath = getFullPathByCheckingParent("Repository-Client-Project-3");

            dlg.InitialDirectory = srcCodeDirPath;
            //dlg.Filter = "cpp|*.cpp|header|*.h|c#|*cs";
            if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                String filename = dlg.SafeFileName;
                Jackson_Info_TextBlock.Text = dlg.SafeFileName;
                Jackson.shim.handDownPostFileMessage(filename, "localhost:" + Jackson.toPort.ToString());

                /// must send a quit to tell code publisher create database!
                //Jackson.shim.handDownPostMessage("quit", "localhost:" + Jackson.toPort.ToString());
            }
        }
        /// <summary>
        /// get all category 
        /// </summary>
        private void Jackson_Btn_GetCateg_Click(object sender, RoutedEventArgs e)
        {
            Jackson_Info_TextBlock.Text = "sent a category request message to server";

            Jackson.shim.handDownGetFileMessage("listall", "localhost:" + Jackson.toPort.ToString());
        }

        /// <summary>
        ///   browsing htms
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>

        private void Jackson_Btn_Blowser_Click(object sender, RoutedEventArgs e)
        {
            string rcvDirPath = getFullPathByCheckingParent("Repository-Server-Project-3");

            foreach (var filename in Jackson_DownloadList.Items)
            {
                string brws = rcvDirPath + @"\" + filename + ".htm";
                System.Diagnostics.Process.Start(brws);
            }
        }

        /// <summary>
        ///  category item selected
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Jackson_CategoryList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox sdr = (ListBox)sender;
            if (sdr.Items.Count == 0) return;
            var endpoint = sdr.SelectedValue;
            Jackson.shim.handDownGetFileMessage(endpoint.ToString(), "localhost:" + Jackson.toPort.ToString());

            Jackson_Info_TextBlock.Text = "category: " + endpoint + " selected,  list its all files";
            Jackson_FileList.Items.Clear();
        }

        /// <summary>
        ///  file list item selected
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Jackson_FileList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (((ListBox)sender).Items.Count == 0) return;

            String filename = (String)((ListBox)sender).SelectedValue;

            Jackson_Info_TextBlock.Text = "download file " + filename + " and its dependency.   You can browze webpage by click button 'Start Broswer'";
            Jackson.shim.handDownGetFileMessage(filename, "localhost:" + Jackson.toPort.ToString());

            Jackson_DownloadList.Items.Clear();
        }

        #endregion

        #region Path Helper,  Initialize Helper

        // Get FullSpec Path by directory Name
        private String getFullPathByCheckingParent( String dirName )
        {
            
            var path = System.IO.Path.GetDirectoryName(Assembly.GetEntryAssembly().Location);

            foreach ( var subDirName in Directory.GetDirectories(path))
            {
                if( subDirName == dirName )
                {
                    return path + @"\" + dirName;
                }
            }

            string currentDir;
            do
            {
                currentDir = Directory.GetParent(path).Name;
                path = Directory.GetParent(path).FullName;
            } while ("Remote_CodePublisher" != currentDir);
            return path+@"\"+dirName;
        }

        /// <summary>
        ///  get target full path by checking down to sub directory
        /// </summary>
        ///
        private String getFullPathByCheckingSubDir(String dirName)
        {
            var nowPath = getFullPathByCheckingParent("Repository-Client-Project-3");
            Queue<String> dirQueue = new Queue<string>();
            dirQueue.Enqueue(nowPath);
            while (dirQueue.Count > 0)
            {
                nowPath = dirQueue.Dequeue();

                var subDirName = nowPath.Substring(nowPath.LastIndexOf('\\') + 1);

                if (subDirName == dirName) return nowPath;

                foreach (var subDir in Directory.GetDirectories(nowPath))
                {
                    dirQueue.Enqueue(subDir);
                }
            }
            return null;
        }


        /// <summary>
        ///  initialize client and start child thread
        /// </summary>
        private void InitializeFawcett()
        {
            Fawcett = new Client()
            {
                clientId = "Fawcett",
                myPort = 8081,
                toPort = 8080
            };
            Fawcett.shim = new Shim(Fawcett.clientId, (uint)Fawcett.myPort, getFullPathByCheckingParent("Repository-Client-Project-3"), getFullPathByCheckingParent("Repository-Server-Project-3"));
            Fawcett.myThread = new Thread(() =>
            {
                int tId = Thread.CurrentThread.ManagedThreadId;
                Dispatcher.Invoke(() =>
                {
                    Fawcett_GroupBox.Header += ", thread id = "+tId.ToString() + ", ip = localhost:" + Fawcett.myPort;
                });
                while ( true )
                {
                    Message Msg = Fawcett.shim.handUpFileMessage(); 
                 
                    if( Msg.command=="post")
                    {
                        if(Msg.filename != null)
                        {  // files download
                            String fnameNohtm = Msg.filename.Substring(0, Msg.filename.LastIndexOf(".htm"));
                            Dispatcher.Invoke(
                                () =>{
                                    Fawcett_DownloadList.Items.Insert(0, fnameNohtm);
                                });
                        }
                        else
                        {
                            if( Msg.msgBody=="localhost:8081" || Msg.msgBody=="localhost:8082")
                            {
                                Dispatcher.Invoke(
                                () => {
                                    Fawcett_CategoryList.Items.Insert(0, Msg.msgBody);
                                });
                            }
                            else if( Msg.msgBody!="quit")
                            {   // file lists
                                if ( Msg.msgBody.IndexOf("demo:") == 0) Dispatcher.Invoke(() => {
                                    Fawcett_DownloadList.Items.Insert(0, Msg.msgBody.Substring(5));
                                });
                                else  Dispatcher.Invoke(() => {
                                    Fawcett_FileList.Items.Insert(0, Msg.msgBody);
                                });     
                            }
                        }  
                    }             
                }
            });
            Fawcett.myThread.Start();
        }

        private void InitializeJackson()
        {
            Jackson = new Client()
            {
                clientId = "Jackson",
                myPort = 8082,
                toPort = 8080
            };
            Jackson.shim = new Shim(Jackson.clientId, (uint)Jackson.myPort, getFullPathByCheckingParent("Repository-Client-Project-3"), getFullPathByCheckingParent("Repository-Server-Project-3"));
            Jackson.myThread = new Thread(() =>
            {
                int tId = Thread.CurrentThread.ManagedThreadId;
                Dispatcher.Invoke(() =>
                {
                    Jackson_GroupBox.Header += ": thread id = " + tId.ToString() + ", ip = localhost:" + Jackson.myPort;
                });
                while (true)
                {
                    Message Msg = Jackson.shim.handUpFileMessage();

                    if (Msg.command == "post")
                    {
                        if (Msg.filename != null)
                        {
                            String fnameNohtm = Msg.filename.Substring(0, Msg.filename.LastIndexOf(".htm"));
                            Dispatcher.Invoke(
                                () =>
                                {
                                    Jackson_DownloadList.Items.Insert(0, fnameNohtm);
                                });
                        }
                        else if( Msg.msgBody!= null)
                        {
                            if (Msg.msgBody == "localhost:8081" || Msg.msgBody == "localhost:8082")
                            {
                                Dispatcher.Invoke(
                                () =>
                                {
                                    Jackson_CategoryList.Items.Insert(0, Msg.msgBody);
                                });
                            }
                            else if (Msg.msgBody != "quit")
                            {
                                if (Msg.msgBody.IndexOf("demo:") == 0) Dispatcher.Invoke(() => {
                                    Jackson_DownloadList.Items.Insert(0, Msg.msgBody.Substring(5));
                                });
                                else Dispatcher.Invoke(() => {
                                    Jackson_FileList.Items.Insert(0, Msg.msgBody);
                                });
                            }
                        }
                    }

                }
            });
            Jackson.myThread.Start();
        }



        #endregion

        #region Tab "Network Confuration"

        // test click  1, JIM start
        private void start_jim_Click(object sender, RoutedEventArgs e)
        {
            if (Jim == null)
            {
                Jim = new Client()
                {
                    clientId = "Jim",
                    myPort = 8091,
                    toPort = 8092
                };
                Jim.shim = new Shim(Jim.clientId, (uint)Jim.myPort, "../Repository-Client-Project-3", "../Repository-Server-Project-3");
                Jim.myThread = new Thread(() =>
                {
                    int tId = Thread.CurrentThread.ManagedThreadId;
                    Dispatcher.Invoke(() =>
                    {
                        jimThreadLabel.Content = tId;
                        jimMyPortLabel.Content = Jim.myPort.ToString();
                        jimToPortLabel.Content = Jim.toPort.ToString();
                    });
                    while (Thread.CurrentThread.IsAlive && Jim!=null)
                    {
                        Message Msg = Jim.shim.handUpFileMessage();
                        Dispatcher.Invoke(() => {
                            
                            listBox_jim.Items.Insert(0, Msg.command + Msg.filename + Msg.msgBody); });
                    }
                });
                Jim.myThread.Start();
            }
            if (Mike == null) start_mike_Click(sender, e);
        }
        // testing click 2, send a mock file request message
        private void test_Jim_Click(object sender, RoutedEventArgs e)
        { 
            if (Jim!=null)
                Jim.shim.handDownGetFileMessage(String.Format("<TEST> client - {0} with myAddr: {1} to address: {2} </TEST>", Jim.clientId, Jim.myPort, Jim.toPort), "localhost:" + Jim.toPort.ToString());
        }
        // test click 3,  stop
        private void stop_jim_Click(object sender, RoutedEventArgs e)
        {
            if(Jim!=null)
            {
                Jim.shim.handDownGetFileMessage("quit", "localhost:" + Jim.toPort.ToString());
            }
        }
        //----------
        // test click 1, start MIKE
        private void start_mike_Click(object sender, RoutedEventArgs e)
        {
            if (this.Mike == null)
            {
                Mike = new Client()
                {
                    clientId = "Mike",
                    myPort = 8092,
                    toPort = 8091
                };
                Mike.shim = new Shim(Mike.clientId, (uint)Mike.myPort, "../Repository-Client-Project-3", "../Repository-Server-Project-3");
                Mike.myThread = new Thread(() =>
                {
                    int tId = Thread.CurrentThread.ManagedThreadId;
                    Dispatcher.Invoke(() =>
                    {
                        mikeThreadLabel.Content = tId;
                        mikeMyPortLabel.Content = Mike.myPort.ToString();
                        mikeToPortLabel.Content = Mike.toPort.ToString();
                    });
                    while (Thread.CurrentThread.IsAlive && Mike!=null)
                    {
                        Message Msg = Mike.shim.handUpFileMessage();
                        Dispatcher.Invoke(() => { listBox_mike.Items.Insert(0, Msg.command + Msg.filename + Msg.msgBody); });
                    }
                });
            Mike.myThread.Start();
            }
            if( Jim==null) start_jim_Click(sender,e);
        }

        // testing click 2, send a mock file request message
        private void test_Mike_Click(object sender, RoutedEventArgs e)
        {  
            if(Mike!=null)
                Mike.shim.handDownGetFileMessage(String.Format("<TEST> client - {0} with myAddr: {1} to address: {2} </TEST>", Mike.clientId, Mike.myPort, Mike.toPort), "localhost:" + Mike.toPort.ToString());
        }          
        // test click 3 stop
        private void stop_mike_Click(object sender, RoutedEventArgs e)
        {
            if(Mike!=null)
            {
                Mike.shim.handDownPostMessage("quit", "localhost:" + Mike.toPort.ToString());
            }
        }
        #endregion

    }


}
