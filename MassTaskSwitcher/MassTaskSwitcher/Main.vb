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
        Public lock As Boolean
        Public noMod As Boolean
    End Structure

    Public Structure lvItmTag
        Public lock As Boolean
        Public noMod As Boolean
    End Structure

    ' Declare globals
    Dim tsTaskDict As New Dictionary(Of String, simpleTask) ' current set of tasks in the task scheduler
    Dim lvTaskDict As New Dictionary(Of String, simpleTask) ' current set of tasks in the List View
    Public lockedTasks As New ArrayList ' The list of tasks we should not be able to enable or disable
    Dim AppFolder As String ' Name of the folder in which the app is located
    Dim lockedTasksFilePath As String ' name of the file containing the saved lockedTasks
    Dim enableCheckEvent As Boolean ' This is used to enable or disable *only* the ListView_check event, when a ListViewItem is being added or updated.
    Dim DisableCheckEvents As Boolean ' This is used to disable the ListView_check and _checked events when the form is intializing
    Dim isV2 As Boolean
    Dim oldLiLabel As String ' When a ListViewItem label is changed, the original label is stored in this value
    Dim tmpFiles As New Dictionary(Of String, String) ' Exported tasks and corresponding temporary XML filenames, used for restoring all tasks to the state as of opening the tool
    Dim nonExportedTasks As New ArrayList ' Array of tasks that could not be exported. Make sure we don't delete these tasks if we do an "undo"


    ' Form class constructor--use to intialize globals
    Public Sub New()

        ' This call is required by the designer.
        InitializeComponent()

        ' Add any initialization after the InitializeComponent() call.
        AppFolder = IO.Path.GetDirectoryName(Application.ExecutablePath)
        lockedTasksFilePath = System.IO.Path.Combine(AppFolder, "lockedTasks.txt")
        enableCheckEvent = True ' Do not allow the tag="locked" ListViewItems to be checked.
        Using ts As New TaskService
            isV2 = (ts.HighestSupportedVersion >= New Version(1, 2))
        End Using

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

        ' Allow F2 to be intercepted in a KeyUp event
        Me.KeyPreview = True

        ' Allow ListViewItem labels to be edited
        lvTasks.LabelEdit = True

        ' Set up the gui combo box
        If cbTsGuiType.Items.Count > 0 Then
            cbTsGuiType.SelectedIndex = 0
        End If
        cbTsGuiType.Visible = Not isV2


        ReadLockedTasksFile() ' Get the names of all the tasks that should be locked.
        InitListView()

        Timer1.Interval = 200 ' Check for updates every 200 ms
        Timer1.Start()
        ExportAllTasks()
    End Sub

    ' "Enable all" button is clicked
    Private Sub btnEnableAll_Click(sender As System.Object, e As System.EventArgs) Handles btnEnableAll.Click
        Dim logEntry As String = InputBox("If you would like to leave a message in the DAQ log explaining this, enter the message and click OK. Click Cancel to leave no message.", "Log Entry", "All radar tasks disabled for debugging, testing or maintenance.")
        If logEntry.Length > 0 Then
            WriteDaqLogEntry(logEntry)
        End If
        ' Find each unchecked ListViewItem, then check it and enable the corresponding task
        Using ts As New TaskService
            For Each li As ListViewItem In lvTasks.Items
                If Not CType(li.Tag, lvItmTag).lock Then
                    If Not li.Checked Then
                        ' Note: We do *not* disable the filtering of the Check event here. Clicking "Enable All" should not enable the "locked" items
                        li.Checked = True
                        Try
                            ts.RootFolder.Tasks.Item(li.Name).Enabled = li.Checked
                        Catch noAccess As UnauthorizedAccessException
                            ' If we can't change a task, just send a message to the user
                            MsgBox("You do not have access to enable or disable the task " + li.Name + ".")
                        Catch ex As Exception
                            ' Rethrow other errors
                            Throw ex
                        End Try
                    End If
                End If
            Next
        End Using
    End Sub

    ' "Disable all" button is clicked
    Private Sub btnDisableAll_Click(sender As System.Object, e As System.EventArgs) Handles btnDisableAll.Click
        Dim logEntry As String = InputBox("If you would like to leave a message in the DAQ log explaining this, enter the message and click OK. Click Cancel to leave no message.", "Log Entry", "All radar tasks disabled for debugging, testing or maintenance.")
        If logEntry.Length > 0 Then
            WriteDaqLogEntry(logEntry)
        End If
        ' Find each unchecked ListViewItem, then check it and enable the corresponding task
        Using ts As New TaskService
            For Each li As ListViewItem In lvTasks.Items
                If Not CType(li.Tag, lvItmTag).lock Then
                    If li.Checked Then
                        ' Note: We do *not* disable the filtering of the Check event here. Clicking "Disable All" should not disable the "locked" items
                        li.Checked = False
                        Try
                            ts.RootFolder.Tasks.Item(li.Name).Enabled = li.Checked
                        Catch noAccess As UnauthorizedAccessException
                            ' If we can't change a task, just send a message to the user
                            MsgBox("You do not have access to enable or disable the task " + li.Name + ".")
                        Catch ex As Exception
                            ' Rethrow other errors
                            Throw ex
                        End Try
                    End If
                End If
            Next
        End Using
    End Sub

    ' "Select Tasks to Not Change" button is clicked
    Private Sub btnSelectTasksToLock_Click(sender As System.Object, e As System.EventArgs) Handles btnSelectTasksToLock.Click
        SelectTasksToLock.ShowDialog()
    End Sub

    ' "Close" button is clicked
    Private Sub btnClose_Click(sender As System.Object, e As System.EventArgs) Handles btnClose.Click
        WriteLockedTasksFile()
        Me.Close()
    End Sub

    ' ListView item checkbox is clicked (before check)
    Private Sub lvTasks_ItemCheck(sender As Object, e As System.Windows.Forms.ItemCheckEventArgs) Handles lvTasks.ItemCheck
        ' there are some checks in here to make sure that this is not run when the list is initializing or when a list item is filtered.

        If Not DisableCheckEvents Then
            If enableCheckEvent And CType(lvTasks.Items(e.Index).Tag, lvItmTag).lock Then
                e.NewValue = e.CurrentValue
            End If
        End If
    End Sub

    ' ListView item checkbox is clicked (after check)
    Private Sub lvTasks_ItemChecked(sender As Object, e As System.Windows.Forms.ItemCheckedEventArgs) Handles lvTasks.ItemChecked
        ' There is a check here to make sure this is not run when the list is intializing, because if it does it causes runtime errors.
        If Not DisableCheckEvents Then
            Using ts As New TaskService
                Try
                    ts.RootFolder.Tasks.Item(e.Item.Name).Enabled = e.Item.Checked
                Catch noAccess As UnauthorizedAccessException
                    ' If we can't change a task, just send a message to the user
                    MsgBox("You do not have access to enable or disable the task " + e.Item.Name + ".")
                Catch ex As Exception
                    ' Rethrow other errors
                    Throw ex
                End Try
            End Using
        End If
    End Sub

    ' A key is released while an item in the ListView is selected
    Private Sub lvTasks_KeyUp(sender As Object, e As System.Windows.Forms.KeyEventArgs) Handles lvTasks.KeyUp
        If e.KeyData = Keys.F2 Then
            ' If it's F2, then edit that item
            For Each li As ListViewItem In lvTasks.SelectedItems
                li.BeginEdit()
            Next
        End If
    End Sub

    ' A label is about to be edited
    Private Sub lvTasks_BeforeLabelEdit(sender As Object, e As System.Windows.Forms.LabelEditEventArgs) Handles lvTasks.BeforeLabelEdit
        If CType(lvTasks.Items(e.Item).Tag, lvItmTag).lock Then
            ' Don't edit if this is a locked task
            e.CancelEdit = True
        Else
            ' Save the existing label
            oldLiLabel = lvTasks.Items(e.Item).Text
        End If
    End Sub

    ' A label has just been edited
    Private Sub lvTasks_AfterLabelEdit(sender As Object, e As System.Windows.Forms.LabelEditEventArgs) Handles lvTasks.AfterLabelEdit
        ' We want to rename the task
        Using ts As New TaskService
            Try
                ' We have to export the task, delete it, and then import it with the new name
                Dim tempFileName As String = System.IO.Path.GetTempFileName
                Dim taskPath As String = ts.RootFolder.Tasks(oldLiLabel).Path
                ts.RootFolder.Tasks(oldLiLabel).Export(tempFileName)
                ts.RootFolder.DeleteTask(oldLiLabel)
                ts.RootFolder.ImportTask(e.Label, tempFileName)
                System.IO.File.Delete(tempFileName)
            Catch uaex As UnauthorizedAccessException
                MsgBox("You do not have permission to modify the task " + oldLiLabel + ".")
                e.CancelEdit = True
            Catch ex As Exception
                ' Rethrow any other exceptions
                Throw ex
            End Try
        End Using
    End Sub


    Private Sub btnEditSelected_Click(sender As System.Object, e As System.EventArgs) Handles btnEditSelected.Click
        If isV2 Or cbTsGuiType.SelectedItem.ToString = "Windows 7-style GUI" Then
            Using ts As New TaskService, tskEdDlg As New TaskEditDialog()
                tskEdDlg.Editable = True
                tskEdDlg.RegisterTaskOnAccept = True
                For Each li As ListViewItem In lvTasks.SelectedItems
                    Try
                        tskEdDlg.Initialize(ts.RootFolder.Tasks.Item(li.Name))
                        tskEdDlg.ShowDialog()
                    Catch uaex As UnauthorizedAccessException
                        MsgBox("You do not have permission to modify the task " + li.Name + ".")
                    Catch ex As Exception
                        ' Rethrow other exceptions
                        Throw ex
                    End Try
                Next
            End Using
        Else
            Using ts As New TaskService
                For Each li As ListViewItem In lvTasks.SelectedItems
                    ts.RootFolder.Tasks.Item(li.Name).ShowPropertyPage()
                Next
            End Using
        End If
    End Sub

    Private Sub btnOpenTaskScheduler_Click(sender As System.Object, e As System.EventArgs) Handles btnOpenTaskScheduler.Click
        Process.Start("control.exe", "schedtasks")
    End Sub

    Private Sub btnDeleteSelected_Click(sender As System.Object, e As System.EventArgs) Handles btnDeleteSelected.Click
        Using ts As New TaskService
            For Each li As ListViewItem In lvTasks.SelectedItems
                Try
                    If MsgBox("Are you sure you want to delete this task? " + vbCrLf + li.Name, MsgBoxStyle.YesNo, "Delete Task?") = MsgBoxResult.Yes Then
                        ts.RootFolder.DeleteTask(li.Name)
                    End If
                Catch uaex As UnauthorizedAccessException
                    MsgBox("You do not have permission to modify the task " + li.Name + ".")
                Catch ex As Exception
                    ' Rethrow other exceptions
                    Throw ex
                End Try
            Next
        End Using
    End Sub

    ' Timer1 tick elapses
    Private Sub Timer1_Tick(sender As System.Object, e As System.EventArgs) Handles Timer1.Tick
        UpdateList()
    End Sub

    Private Sub Main_FormClosed(sender As Object, e As System.Windows.Forms.FormClosedEventArgs) Handles Me.FormClosed
        ' Delete the temporary files
        For Each tmpFile As String In tmpFiles.Values
            System.IO.File.Delete(tmpFile)
        Next
    End Sub

    ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    ' Subs and functions
    ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

    ' Update the global tsTaskDict with the tasks from the Task Scheduler
    Private Sub GetTaskServiceTasks()
        tsTaskDict.Clear()
        Using ts As New TaskService
            For Each tsk As Task In ts.RootFolder.Tasks
                Dim st As simpleTask
                st.name = tsk.Name
                st.state = tsk.State
                st.enabled = tsk.Enabled
                st.lastRun = tsk.LastRunTime
                st.nextRun = tsk.NextRunTime
                ' Simple test to see if we have privileges to modify the task
                Try
                    st.noMod = False
                    ' Try setting the task's Enabled state to itself. Doesn't change the task, but tests access.
                    tsk.Enabled = tsk.Enabled
                Catch uaex As UnauthorizedAccessException
                    ' If not, then it may not be modified
                    st.noMod = True
                Catch ex As Exception
                    ' Rethrow other exceptions
                    Throw ex
                End Try

                If lockedTasks.Contains(tsk.Name) Or st.noMod Then
                    st.lock = True
                Else
                    st.lock = False
                End If
                tsTaskDict.Add(tsk.Name, st)
            Next
        End Using
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
            st.lock = CType(itm.Tag, lvItmTag).lock
            st.noMod = CType(itm.Tag, lvItmTag).noMod
            lvTaskDict.Add(st.name, st)
        Next
    End Sub

    ' Re-initialize the listView with the current set of tasks
    Public Sub InitListView()
        GetTaskServiceTasks()
        DisableCheckEvents = True
        lvTasks.Items.Clear()
        For Each st As simpleTask In tsTaskDict.Values
            AddNewListItem(st)
        Next
        DisableCheckEvents = False
    End Sub

    ' Update the List View with the current set of tasks. Allows us to get away with not re-initializing the entire list all the time
    Private Sub AddNewListItem(st As simpleTask)
        Dim li As New ListViewItem
        li = New ListViewItem({st.name, st.state.ToString, DateToString(st.lastRun), DateToString(st.nextRun)})
        li.Checked = st.enabled
        li.Name = st.name
        li.Tag = New lvItmTag With {.lock = st.lock, .noMod = st.noMod}
        If st.lock Then
            li.BackColor = Color.DarkGray
        Else
            li.BackColor = Color.White
        End If
        enableCheckEvent = False ' Disable the filtering of the "check" event for the "locked" tag
        lvTasks.Items.Add(li)
        enableCheckEvent = True
    End Sub

    ' Check for differences between the GUI task list and the tasks in the Task Scheduler service, and update the GUI if changes have been made.
    Private Sub UpdateList()
        GetListViewTasks() ' update the list of tasks currently in the GUI
        GetTaskServiceTasks() ' update the list of tasks currently in the Task Scheduler service
        If tsTaskDict.Count <> lvTaskDict.Count Then
            ' The count of tasks has changed. Reinit the whole list
            InitListView()
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
                    InitListView()
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
        enableCheckEvent = False ' Disable the filtering of the "check" event for the "locked" tag
        For Each li As ListViewItem In lvTasks.Items.Find(st.name, True)
            li.Checked = st.enabled
            li.SubItems(0).Text = st.name
            li.SubItems(1).Text = st.state.ToString
            li.SubItems(2).Text = DateToString(st.lastRun)
            li.SubItems(3).Text = DateToString(st.nextRun)
        Next
        enableCheckEvent = True
    End Sub

    ' Read in the file (if found) that contains the tasks we should not be able to enable or disable
    Private Sub ReadLockedTasksFile()
        lockedTasks.Clear()
        If System.IO.File.Exists(lockedTasksFilePath) Then
            Using sr As New System.IO.StreamReader(lockedTasksFilePath)
                While Not sr.EndOfStream
                    lockedTasks.Add(sr.ReadLine())
                End While
            End Using
        Else
            ' Do nothing. no lockedTasks file to load
            Debug.Print("lockedTasks file not found'")
        End If

    End Sub

    ' Write the file that contains the tasks we should not be able to enable or disable
    Private Sub WriteLockedTasksFile()
        Dim sb As New System.Text.StringBuilder()
        For Each tsk As String In lockedTasks
            sb.AppendLine(tsk)
        Next
        Using outfile As New IO.StreamWriter(lockedTasksFilePath, False)
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

    Private Sub ExportAllTasks()
        ' Note: This will only export tasks that are modifiable by the current user
        tmpFiles.Clear()
        Using ts As New TaskService
            For Each tsk As Task In ts.RootFolder.Tasks
                Try
                    ' See if the task is modifiable
                    tsk.Enabled = tsk.Enabled
                Catch uaex As UnauthorizedAccessException
                    ' If it is not modifiable, don't save it.
                    Continue For
                Catch ex As Exception
                    ' Rethrow other exceptions
                    Throw (ex)
                End Try
                ' Save the name of the task and a new tempfile name for the task to the tmpFiles dictionary
                tmpFiles.Add(tsk.Name, System.IO.Path.GetTempFileName)
                ' Export the task to a new tempfile
                Try
                    tsk.Export(tmpFiles(tsk.Name))
                Catch nrfex As NullReferenceException
                    nonExportedTasks.Add(tsk.Name)
                Catch ex As Exception
                    ' Rethrow other exceptions
                    Throw (ex)
                End Try
            Next
        End Using
    End Sub

    Private Sub ResetTasksByImport()
        Using ts As New TaskService
            ' Delete all modifiable tasks
            For Each tsk As Task In ts.RootFolder.Tasks
                Try
                    ' See if the task is modifiable
                    tsk.Enabled = tsk.Enabled
                Catch uaex As UnauthorizedAccessException
                    ' If it is not modifiable, don't delete it.
                    Continue For
                Catch ex As Exception
                    ' Rethrow other exceptions
                    Throw (ex)
                End Try
                If Not nonExportedTasks.Contains(tsk.Name) Then
                    ' Don't delete it if we couldn't export it
                    ts.RootFolder.DeleteTask(tsk.Name)
                End If
            Next

            ' Now import the exported tasks
            For Each kvp As KeyValuePair(Of String, String) In tmpFiles
                ts.RootFolder.ImportTask(kvp.Key, kvp.Value)
            Next

        End Using
    End Sub


    Private Sub btnRevertAllChanges_Click(sender As System.Object, e As System.EventArgs) Handles btnRevertAllChanges.Click
        If MsgBox("Are you sure you want to revert all changes?", MsgBoxStyle.YesNo, "Revert changes?") = MsgBoxResult.Yes Then
            ResetTasksByImport()
        End If
    End Sub

End Class