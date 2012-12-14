Imports System.Text.RegularExpressions
Imports Microsoft.Win32.TaskScheduler
Imports System.Collections

Public Class old_form
    'Structure Def
    Private Structure TaskInfo
        Public State As TaskState
        Public Enabled As Boolean
    End Structure

    ' Form globals
    Dim lastTasks As New Dictionary(Of String, TaskInfo)
    Dim currentTasks As New Dictionary(Of String, TaskInfo)

    ' Form Load function
    Private Sub Form1_Load(sender As Object, e As System.EventArgs) Handles Me.Load
        Timer1.Interval = 200
        Timer1.Start()
    End Sub

    Private Sub Timer1_Tick(sender As Object, e As System.EventArgs) Handles Timer1.Tick
        UpdateListView()
    End Sub

    Private Sub UpdateListView()
        ' Declarations
        Dim ts As New TaskService
        Dim rePopulate As Boolean
        Dim changedTasks() As String
        Dim chgCount As Integer

        ' Inits
        chgCount = 0
        changedTasks = {}

        ' First, we save the currentTasks to lastTasks and get the current tasks
        lastTasks.Clear()
        If currentTasks.Count > 0 Then
            For Each pair As KeyValuePair(Of String, TaskInfo) In currentTasks
                lastTasks.Add(pair.Key, pair.Value)
            Next
        End If
        currentTasks.Clear()
        For Each tsk As Task In ts.RootFolder.Tasks
            Dim ti As TaskInfo
            ti.Enabled = tsk.Enabled
            ti.State = tsk.State
            currentTasks.Add(tsk.Name, ti)
        Next


        If currentTasks.Count > 0 And (currentTasks.Count = lastTasks.Count) Then
            ReDim changedTasks(currentTasks.Count) ' allocate enough space for all strings
            ' Next we want to see if any tasks have been added or removed
            For Each pair As KeyValuePair(Of String, TaskInfo) In currentTasks
                Try
                    Dim lastDictMatch As TaskInfo = lastTasks(pair.Key)
                    If (lastDictMatch.Enabled <> pair.Value.Enabled) Or (lastDictMatch.State <> pair.Value.State) Then
                        changedTasks(chgCount) = pair.Key
                        chgCount = chgCount + 1
                    End If
                Catch ex As Exception
                    ' The name of one of the current tasks does not match any of the former tasks. Repopulate the ListView completely
                    rePopulate = True
                End Try
            Next
            If chgCount > 0 Then
                ReDim Preserve changedTasks(chgCount - 1) ' now trim array to actual size
            End If
        Else
            ' Either there are no tasks or the number of tasks has changed. Repopulate the ListView completely
            rePopulate = True
        End If

        If rePopulate Then
            lstVwTasks.Items.Clear()
            For Each tsk As Task In ts.RootFolder.Tasks
                Try
                    Dim lvitm As New ListViewItem
                    lvitm = New ListViewItem({tsk.Name, tsk.State.ToString})
                    lvitm.Name = tsk.Name
                    lvitm.Checked = tsk.Enabled
                    lstVwTasks.Items.Add(lvitm)
                Catch ex As Exception
                    Dim x As Integer
                End Try
            Next
        ElseIf chgCount > 0 Then
            For Each tskName As String In changedTasks
                For Each itm As ListViewItem In lstVwTasks.Items.Find(tskName, True)
                    itm.SubItems(1).Text = currentTasks(tskName).State.ToString
                    itm.Checked = currentTasks(tskName).Enabled
                Next
            Next
        End If


    End Sub

End Class
