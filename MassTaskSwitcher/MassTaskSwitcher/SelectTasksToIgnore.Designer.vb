<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class SelectTasksToIgnore
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
        Me.btnOK = New System.Windows.Forms.Button()
        Me.btnCancel = New System.Windows.Forms.Button()
        Me.lvIgnoredTasks = New System.Windows.Forms.ListView()
        Me.taskName = CType(New System.Windows.Forms.ColumnHeader(), System.Windows.Forms.ColumnHeader)
        Me.SuspendLayout()
        '
        'btnOK
        '
        Me.btnOK.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnOK.Location = New System.Drawing.Point(12, 218)
        Me.btnOK.Name = "btnOK"
        Me.btnOK.Size = New System.Drawing.Size(89, 33)
        Me.btnOK.TabIndex = 1
        Me.btnOK.Text = "OK"
        Me.btnOK.UseVisualStyleBackColor = True
        '
        'btnCancel
        '
        Me.btnCancel.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.btnCancel.Location = New System.Drawing.Point(362, 218)
        Me.btnCancel.Name = "btnCancel"
        Me.btnCancel.Size = New System.Drawing.Size(89, 33)
        Me.btnCancel.TabIndex = 1
        Me.btnCancel.Text = "Cancel"
        Me.btnCancel.UseVisualStyleBackColor = True
        '
        'lvIgnoredTasks
        '
        Me.lvIgnoredTasks.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.lvIgnoredTasks.CheckBoxes = True
        Me.lvIgnoredTasks.Columns.AddRange(New System.Windows.Forms.ColumnHeader() {Me.taskName})
        Me.lvIgnoredTasks.Location = New System.Drawing.Point(12, 12)
        Me.lvIgnoredTasks.Name = "lvIgnoredTasks"
        Me.lvIgnoredTasks.Size = New System.Drawing.Size(438, 194)
        Me.lvIgnoredTasks.TabIndex = 2
        Me.lvIgnoredTasks.UseCompatibleStateImageBehavior = False
        Me.lvIgnoredTasks.View = System.Windows.Forms.View.Details
        '
        'taskName
        '
        Me.taskName.Text = "Task"
        Me.taskName.Width = 434
        '
        'SelectTasksToIgnore
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(467, 263)
        Me.Controls.Add(Me.lvIgnoredTasks)
        Me.Controls.Add(Me.btnCancel)
        Me.Controls.Add(Me.btnOK)
        Me.Name = "SelectTasksToIgnore"
        Me.Text = "SelectTasksToIgnore"
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents btnOK As System.Windows.Forms.Button
    Friend WithEvents btnCancel As System.Windows.Forms.Button
    Friend WithEvents lvIgnoredTasks As System.Windows.Forms.ListView
    Friend WithEvents taskName As System.Windows.Forms.ColumnHeader
End Class
