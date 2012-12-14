<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class old_form
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Me.lstVwTasks = New System.Windows.Forms.ListView()
        Me.taskName = CType(New System.Windows.Forms.ColumnHeader(), System.Windows.Forms.ColumnHeader)
        Me.taskState = CType(New System.Windows.Forms.ColumnHeader(), System.Windows.Forms.ColumnHeader)
        Me.Timer1 = New System.Windows.Forms.Timer(Me.components)
        Me.SuspendLayout()
        '
        'lstVwTasks
        '
        Me.lstVwTasks.CheckBoxes = True
        Me.lstVwTasks.Columns.AddRange(New System.Windows.Forms.ColumnHeader() {Me.taskName, Me.taskState})
        Me.lstVwTasks.GridLines = True
        Me.lstVwTasks.Location = New System.Drawing.Point(40, 23)
        Me.lstVwTasks.Name = "lstVwTasks"
        Me.lstVwTasks.Size = New System.Drawing.Size(480, 224)
        Me.lstVwTasks.TabIndex = 0
        Me.lstVwTasks.UseCompatibleStateImageBehavior = False
        Me.lstVwTasks.View = System.Windows.Forms.View.Details
        '
        'taskName
        '
        Me.taskName.Text = "Task Name"
        Me.taskName.Width = 89
        '
        'taskState
        '
        Me.taskState.Text = "State"
        Me.taskState.Width = 52
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(557, 272)
        Me.Controls.Add(Me.lstVwTasks)
        Me.Name = "Form1"
        Me.Text = "Form1"
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents lstVwTasks As System.Windows.Forms.ListView
    Friend WithEvents taskName As System.Windows.Forms.ColumnHeader
    Friend WithEvents taskState As System.Windows.Forms.ColumnHeader
    Friend WithEvents Timer1 As System.Windows.Forms.Timer

End Class
