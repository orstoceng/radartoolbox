Option Explicit On
Option Strict On

Imports Microsoft.Win32.TaskScheduler
Imports System.Collections

Public Class Main
    ' Struct dfn
    Private Structure simpleTask
        Public name As String
        Public state As TaskState
        Public enabled As Boolean
        Public lastRun As Date
        Public nextRun As Date
        Public ignore As Boolean
    End Structure

    ' Declare globals
    Dim tsTaskDict As New Dictionary(Of String, simpleTask) ' current set of tasks in the task scheduler
    Dim lvTaskDict As New Dictionary(Of String, simpleTask) ' current set of tasks in the List View
    Public ignoredTasks As New ArrayList ' The list of tasks we should not be able to enable or disable
    Dim AppFolder As String ' Name of the folder in which the app is located
    Dim ignoredTasksFilePath As String ' name of the file containing the saved ignoredTasks
    Dim filterCheckEvent As Boolean ' If True, then the ListView.ItemCheck event will check for "Ignore" in the ListViewItem's Tag before allowing the check
    Dim DisableCheckChecker As Boolean

    ' Form class constructor--use to intialize globals
    Public Sub New()

        ' This call is required by the designer.
        InitializeComponent()

        ' Add any initialization after the InitializeComponent() call.
        AppFolder = IO.Path.GetDirectoryName(Application.ExecutablePath)
        ignoredTasksFilePath = System.IO.Path.Combine(AppFolder, "ignoredTasks.txt")
        filterCheckEvent = True ' Do not allow the tag="ignore" ListViewItems to be checked.

    End Sub


    ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    ' Events
    ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

    ' Form Load event
    Private Sub Main_Load(sender As Object, e As System.EventArgs) Handles Me.Load
        ' Set up the position of the form on the screen
        Dim maxX, maxY, centerX, centerY As Integer
        maxX = Screen.PrimaryScreen.WorkingArea.Width - Me.Width
        maxY = Screen.PrimaryScreen.WorkingArea.Height - Me.Height
        centerX = CInt(Screen.PrimaryScreen.WorkingArea.Width / 2 - Me.Width / 2)
        centerY = CInt(Screen.PrimaryScreen.WorkingArea.Height / 2 - Me.Height / 2)
        For Each arg As String In My.Application.CommandLineArgs
            Select Case arg
                Case "-tl"
                    Me.Location = New Point(0, 0)
                Case "-tr"
                    Me.Location = New Point(maxX, 0)
                Case "-bl"
                    Me.Location = New Point(0, maxY)
                Case "-br"
                    Me.Location = New Point(maxX, maxY)
                Case "-center"
                    Me.Location = New Point(centerX, centerY)
                Case Else
                    MsgBox("Invalid input argument: " + arg)
                    Me.Close()
            End Select
        Next

        ReadIgnoredTasksFile()
        InitList()
        Timer1.Interval = 200 ' Check for updates ever 200 ms
        Timer1.Start()
    End Sub

    ' "Enable all" button is clicked
    Private Sub btnEnableAll_Click(sender As System.Object, e As System.EventArgs) Handles btnEnableAll.Click
        Dim logEntry As String = InputBox("If you would like to leave a message in the DAQ log explaining this, enter the message and click OK. Click Cancel to leave no message.", "Log Entry", "All radar tasks disabled for debugging, testing or maintenance.")
        If logEntry.Length > 0 Then
            WriteDaqLogEntry(logEntry)
        End If
        ' Find each unchecked ListViewItem, then check it and enable the corresponding task
        Dim ts As New TaskService
        For Each li As ListViewItem In lvTasks.Items
            If Not li.Tag.Equals("ignore") Then
                If Not li.Checked Then
                    ' Note: We do *not* disable the filtering of the Check event here. Clicking "Enable All" should not enable the "ignored" items
                    li.Checked = True
                    Try
                        ts.RootFolder.Tasks.Item(li.Name).Enabled = li.Checked
                    Catch noAccess As System.UnauthorizedAccessException
                        ' If we can't change a task, just send a message to the user
                        MsgBox("You do not have access to enable or disable the task " + li.Name + ".")
                    Catch ex As Exception
                        ' Rethrow other errors
                        Throw ex
                    End Try
                End If
            End If
        Next
    End Sub

    ' "Disable all" button is clicked
    Private Sub btnDisableAll_Click(sender As System.Object, e As System.EventArgs) Handles btnDisableAll.Click
        Dim logEntry As String = InputBox("If you would like to leave a message in the DAQ log explaining this, enter the message and click OK. Click Cancel to leave no message.", "Log Entry", "All radar tasks disabled for debugging, testing or maintenance.")
        If logEntry.Length > 0 Then
            WriteDaqLogEntry(logEntry)
        End If
        ' Find each unchecked ListViewItem, then check it and enable the corresponding task
        Dim ts As New TaskService
        For Each li As ListViewItem In lvTasks.Items
            If Not li.Tag.Equals("ignore") Then
                If li.Checked Then
                    ' Note: We do *not* disable the filtering of the Check event here. Clicking "Disable All" should not disable the "ignored" items
                    li.Checked = False
                    Try
                        ts.RootFolder.Tasks.Item(li.Name).Enabled = li.Checked
                    Catch noAccess As System.UnauthorizedAccessException
                        ' If we can't change a task, just send a message to the user
                        MsgBox("You do not have access to enable or disable the task " + li.Name + ".")
                    Catch ex As Exception
                        ' Rethrow other errors
                        Throw ex
                    End Try
                End If
            End If
        Next
    End Sub

    ' "Select Tasks to Not Change" button is clicked
    Private Sub btnIgnoredTasks_Click(sender As System.Object, e As System.EventArgs) Handles btnIgnoredTasks.Click
        SelectTasksToIgnore.ShowDialog()
    End Sub

    ' "Close" button is clicked
    Private Sub btnClose_Click(sender As System.Object, e As System.EventArgs) Handles btnClose.Click
        WriteIgnoredTasksFile()
        Me.Close()
    End Sub

    ' ListView item checkbox is clicked (before check)
    Private Sub lvTasks_ItemCheck(sender As Object, e As System.Windows.Forms.ItemCheckEventArgs) Handles lvTasks.ItemCheck
        ' there are some checks in here to make sure that this is not run when the list is initializing or when a list item is filtered.
        If Not DisableCheckChecker Then
            If filterCheckEvent And (lvTasks.Items(e.Index).Tag.Equals("ignore")) Then
                e.NewValue = e.CurrentValue
            End If
        End If
    End Sub

    ' ListView item checkbox is clicked (after check)
    Private Sub lvTasks_ItemChecked(sender As Object, e As System.Windows.Forms.ItemCheckedEventArgs) Handles lvTasks.ItemChecked
        ' There is a check here to make sure this is not run when the list is intializing, because if it does it causes runtime errors.
        If Not DisableCheckChecker Then
            Dim ts As New TaskService
            Try
                ts.RootFolder.Tasks.Item(e.Item.Name).Enabled = e.Item.Checked
            Catch noAccess As System.UnauthorizedAccessException
                ' If we can't change a task, just send a message to the user
                MsgBox("You do not have access to enable or disable the task " + e.Item.Name + ".")
            Catch ex As Exception
                ' Rethrow other errors
                Throw ex
            End Try
        End If
    End Sub

    ' Timer1 tick elapses
    Private Sub Timer1_Tick(sender As System.Object, e As System.EventArgs) Handles Timer1.Tick
        UpdateList()
    End Sub

    ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    ' Subs and functions
    ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

    ' Update the global tsTaskDict with the tasks from the Task Scheduler
    Private Sub GetTaskServiceTasks()
        tsTaskDict.Clear()
        Dim ts As New TaskService
        For Each tsk As Task In ts.RootFolder.Tasks
            Dim st As simpleTask
            st.name = tsk.Name
            st.state = tsk.State
            st.enabled = tsk.Enabled
            st.lastRun = tsk.LastRunTime
            st.nextRun = tsk.NextRunTime
            If ignoredTasks.Contains(tsk.Name) Or tsk.Definition.Principal.UserId Is Nothing Then
                st.ignore = True
            Else
                st.ignore = False
            End If
            tsTaskDict.Add(tsk.Name, st)
        Next
    End Sub

    ' Get the tasks in the current ListView and save them to lvTaskDict
    Private Sub GetListViewTasks()
        lvTaskDict.Clear()
        For Each itm As ListViewItem In lvTasks.Items
            Dim st As simpleTask
            st.name = itm.Name
            st.state = CType([Enum].Parse(GetType(TaskState), itm.SubItems(1).Text), TaskState)
            st.enabled = itm.Checked
            st.lastRun = StringToDate(itm.SubItems(2).Text)
            st.nextRun = StringToDate(itm.SubItems(3).Text)
            lvTaskDict.Add(st.name, st)
        Next
    End Sub

    ' Re-initialize the listView with the current set of tasks
    Public Sub InitList()
        GetTaskServiceTasks()
        DisableCheckChecker = True
        lvTasks.Items.Clear()
        For Each st As simpleTask In tsTaskDict.Values
            AddNewListItem(st)
        Next
        DisableCheckChecker = False
    End Sub

    ' Update the List View with the current set of tasks. Allows us to get away with not re-initializing the entire list all the time
    Private Sub AddNewListItem(st As simpleTask)
        Dim li As New ListViewItem
        li = New ListViewItem({st.name, st.state.ToString, DateToString(st.lastRun), DateToString(st.nextRun)})
        li.Checked = st.enabled
        li.Name = st.name
        If st.ignore Then
            li.BackColor = Color.DarkGray
            li.Tag = "ignore"
        Else
            li.BackColor = Color.White
            li.Tag = ""
        End If
        filterCheckEvent = False ' Disable the filtering of the "check" event for the "ignore" tag
        lvTasks.Items.Add(li)
        filterCheckEvent = True
    End Sub

    ' Check for differences between the GUI task list and the tasks in the Task Scheduler service, and update the GUI if changes have been made.
    Private Sub UpdateList()
        GetListViewTasks() ' update the list of tasks currently in the GUI
        GetTaskServiceTasks() ' update the list of tasks currently in the Task Scheduler service
        If tsTaskDict.Count <> lvTaskDict.Count Then
            ' The count of tasks has changed. Reinit the whole list
            InitList()
        Else
            ' Go through each task in the Task Scheduler
            For Each tsTD_pair As KeyValuePair(Of String, simpleTask) In tsTaskDict
                Try
                    ' Try to find a matching task in the list of tasks in the GUI.
                    If Not lvTaskDict.Item(tsTD_pair.Key).Equals(tsTD_pair.Value) Then
                        ' If it's found and its value has changed, update it.
                        UpdateListItem(tsTD_pair.Value)
                    End If
                Catch NoMatch As System.Collections.Generic.KeyNotFoundException
                    ' If There is no matching key, an exception will be thrown. If this happens, reinit the whole GUI list.
                    InitList()
                    Exit For
                Catch OtherErrors As Exception
                    ' Some other error occured. Re-throw.
                    Throw OtherErrors
                    Exit For
                End Try
            Next
        End If

    End Sub

    ' Updates each list item whose key matches the simpleTask name to the contents of the simpleTask.
    Private Sub UpdateListItem(st As simpleTask)
        filterCheckEvent = False ' Disable the filtering of the "check" event for the "ignore" tag
        For Each li As ListViewItem In lvTasks.Items.Find(st.name, True)
            li.Checked = st.enabled
            li.SubItems(0).Text = st.name
            li.SubItems(1).Text = st.state.ToString
            li.SubItems(2).Text = DateToString(st.lastRun)
            li.SubItems(3).Text = DateToString(st.nextRun)
        Next
        filterCheckEvent = True
    End Sub

    ' Read in the file (if found) that contains the tasks we should not be able to enable or disable
    Private Sub ReadIgnoredTasksFile()
        ignoredTasks.Clear()
        If System.IO.File.Exists(ignoredTasksFilePath) Then
            Using sr As New System.IO.StreamReader(ignoredTasksFilePath)
                While Not sr.EndOfStream
                    ignoredTasks.Add(sr.ReadLine())
                End While
            End Using
        Else
            ' Do nothing. no ignoredTasks file to load
            Debug.Print("ignoredTasks file not found'")
        End If

    End Sub

    ' Write the file that contains the tasks we should not be able to enable or disable
    Private Sub WriteIgnoredTasksFile()
        Dim sb As New System.Text.StringBuilder()
        For Each tsk As String In ignoredTasks
            sb.AppendLine(tsk)
        Next
        Using outfile As New IO.StreamWriter(ignoredTasksFilePath, False)
            outfile.Write(sb.ToString())
        End Using
    End Sub

    ' convert a date value to a string for display
    Private Function DateToString(dt As Date) As String
        If dt.ToBinary = 0 Then
            Return ""
        Else
            Return dt.ToString("yyyy-MM-dd HH:mm:ss")
        End If
    End Function

    ' convert a displayed date string to a date value
    Private Function StringToDate(str As String) As Date
        If str = "" Then
            Return Date.FromBinary(0)
        Else
            Return Date.ParseExact(str, "yyyy-MM-dd HH:mm:ss", System.Globalization.CultureInfo.CurrentCulture)
        End If
    End Function

    Private Sub WriteDaqLogEntry(logEntry As String)
        Try
            Dim psi As New ProcessStartInfo("C:\1MarineWinXP\other_scripts\custom_daq_log_entry.bat", logEntry)
            psi.RedirectStandardError = True
            psi.RedirectStandardOutput = True
            psi.CreateNoWindow = False
            psi.WindowStyle = ProcessWindowStyle.Hidden
            psi.UseShellExecute = False
            Dim process As Process = process.Start(psi)
        Catch fnf As System.IO.FileNotFoundException
            ' If we don't have the script, catch the exception and move on
            Debug.Print("Cannot write DAQ log entry. Script does not exist")
        Catch w32ex As System.ComponentModel.Win32Exception
            ' If we don't have the script, catch the exception and move on
            Debug.Print("Cannot write DAQ log entry. Script does not exist")
        Catch ex As Exception
            ' Re-throw other exceptions
            Throw ex
        End Try
    End Sub

End Class