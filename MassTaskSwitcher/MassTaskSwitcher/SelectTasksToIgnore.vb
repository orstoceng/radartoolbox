Option Explicit On
Option Strict On

Public Class SelectTasksToLock

    'Use .Shown event instead of .Load event so it runs every time the form opens, not just the first time
    Private Sub SelectTasksToLock_Shown(sender As Object, e As System.EventArgs) Handles Me.Shown
        lvLockedTasks.Items.Clear()
        For Each m_li As ListViewItem In Main.lvTasks.Items
            Dim li As New ListViewItem
            li = New ListViewItem({m_li.Name})
            li.Name = m_li.Name
            If Not m_li.Tag Is Nothing And CType(m_li.Tag, Main.lvItmTag).lock Then
                li.Checked = True
                lvLockedTasks.Items.Add(li)
            Else
                li.Checked = False
                lvLockedTasks.Items.Add(li)
            End If
        Next

    End Sub

    Private Sub btnOK_Click(sender As System.Object, e As System.EventArgs) Handles btnOK.Click
        Main.lockedTasks.Clear()
        Dim count As Integer = 0
        For Each li As ListViewItem In lvLockedTasks.Items
            If li.Checked Then
                Main.lockedTasks.Add(li.Name)
            End If
        Next
        Main.InitListView()
        Me.Close()
    End Sub

    Private Sub btnCancel_Click(sender As System.Object, e As System.EventArgs) Handles btnCancel.Click
        Me.Close()
    End Sub

End Class