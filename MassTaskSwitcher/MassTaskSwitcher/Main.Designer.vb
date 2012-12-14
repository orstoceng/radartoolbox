<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Main
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
        Dim ListViewItem1 As System.Windows.Forms.ListViewItem = New System.Windows.Forms.ListViewItem(New String() {"Default Item Name", "Disabled", "", ""}, -1)
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(Main))
        Me.lvTasks = New System.Windows.Forms.ListView()
        Me.tskName = CType(New System.Windows.Forms.ColumnHeader(), System.Windows.Forms.ColumnHeader)
        Me.tskState = CType(New System.Windows.Forms.ColumnHeader(), System.Windows.Forms.ColumnHeader)
        Me.tskLastRun = CType(New System.Windows.Forms.ColumnHeader(), System.Windows.Forms.ColumnHeader)
        Me.tskNextRun = CType(New System.Windows.Forms.ColumnHeader(), System.Windows.Forms.ColumnHeader)
        Me.Timer1 = New System.Windows.Forms.Timer(Me.components)
        Me.btnDisableAll = New System.Windows.Forms.Button()
        Me.btnClose = New System.Windows.Forms.Button()
        Me.btnEnableAll = New System.Windows.Forms.Button()
        Me.btnIgnoredTasks = New System.Windows.Forms.Button()
        Me.lblInstructions = New System.Windows.Forms.Label()
        Me.SuspendLayout()
        '
        'lvTasks
        '
        Me.lvTasks.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.lvTasks.CheckBoxes = True
        Me.lvTasks.Columns.AddRange(New System.Windows.Forms.ColumnHeader() {Me.tskName, Me.tskState, Me.tskLastRun, Me.tskNextRun})
        Me.lvTasks.GridLines = True
        ListViewItem1.StateImageIndex = 0
        Me.lvTasks.Items.AddRange(New System.Windows.Forms.ListViewItem() {ListViewItem1})
        Me.lvTasks.Location = New System.Drawing.Point(12, 76)
        Me.lvTasks.Name = "lvTasks"
        Me.lvTasks.Size = New System.Drawing.Size(621, 254)
        Me.lvTasks.TabIndex = 0
        Me.lvTasks.UseCompatibleStateImageBehavior = False
        Me.lvTasks.View = System.Windows.Forms.View.Details
        '
        'tskName
        '
        Me.tskName.Text = "Name"
        Me.tskName.Width = 306
        '
        'tskState
        '
        Me.tskState.Text = "State"
        Me.tskState.Width = 43
        '
        'tskLastRun
        '
        Me.tskLastRun.Text = "Last Run"
        Me.tskLastRun.Width = 134
        '
        'tskNextRun
        '
        Me.tskNextRun.Text = "Next Run"
        Me.tskNextRun.Width = 134
        '
        'Timer1
        '
        '
        'btnDisableAll
        '
        Me.btnDisableAll.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnDisableAll.Location = New System.Drawing.Point(131, 336)
        Me.btnDisableAll.Name = "btnDisableAll"
        Me.btnDisableAll.Size = New System.Drawing.Size(111, 37)
        Me.btnDisableAll.TabIndex = 2
        Me.btnDisableAll.Text = "Disable All"
        Me.btnDisableAll.UseVisualStyleBackColor = True
        '
        'btnClose
        '
        Me.btnClose.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.btnClose.Location = New System.Drawing.Point(522, 336)
        Me.btnClose.Name = "btnClose"
        Me.btnClose.Size = New System.Drawing.Size(111, 37)
        Me.btnClose.TabIndex = 2
        Me.btnClose.Text = "Close"
        Me.btnClose.UseVisualStyleBackColor = True
        '
        'btnEnableAll
        '
        Me.btnEnableAll.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnEnableAll.Location = New System.Drawing.Point(14, 336)
        Me.btnEnableAll.Name = "btnEnableAll"
        Me.btnEnableAll.Size = New System.Drawing.Size(111, 37)
        Me.btnEnableAll.TabIndex = 2
        Me.btnEnableAll.Text = "Enable All"
        Me.btnEnableAll.UseVisualStyleBackColor = True
        '
        'btnIgnoredTasks
        '
        Me.btnIgnoredTasks.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnIgnoredTasks.Location = New System.Drawing.Point(248, 336)
        Me.btnIgnoredTasks.Name = "btnIgnoredTasks"
        Me.btnIgnoredTasks.Size = New System.Drawing.Size(158, 37)
        Me.btnIgnoredTasks.TabIndex = 2
        Me.btnIgnoredTasks.Text = "Select Tasks to Not Change"
        Me.btnIgnoredTasks.UseVisualStyleBackColor = True
        '
        'lblInstructions
        '
        Me.lblInstructions.AutoSize = True
        Me.lblInstructions.Location = New System.Drawing.Point(12, 9)
        Me.lblInstructions.Name = "lblInstructions"
        Me.lblInstructions.Size = New System.Drawing.Size(431, 52)
        Me.lblInstructions.TabIndex = 3
        Me.lblInstructions.Text = resources.GetString("lblInstructions.Text")
        '
        'Main
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(645, 382)
        Me.Controls.Add(Me.lblInstructions)
        Me.Controls.Add(Me.btnClose)
        Me.Controls.Add(Me.btnIgnoredTasks)
        Me.Controls.Add(Me.btnDisableAll)
        Me.Controls.Add(Me.btnEnableAll)
        Me.Controls.Add(Me.lvTasks)
        Me.Name = "Main"
        Me.Text = "Mass Task Switcher"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents lvTasks As System.Windows.Forms.ListView
    Friend WithEvents tskName As System.Windows.Forms.ColumnHeader
    Friend WithEvents tskState As System.Windows.Forms.ColumnHeader
    Friend WithEvents tskLastRun As System.Windows.Forms.ColumnHeader
    Friend WithEvents tskNextRun As System.Windows.Forms.ColumnHeader
    Friend WithEvents Timer1 As System.Windows.Forms.Timer
    Friend WithEvents btnDisableAll As System.Windows.Forms.Button
    Friend WithEvents btnClose As System.Windows.Forms.Button
    Friend WithEvents btnEnableAll As System.Windows.Forms.Button
    Friend WithEvents btnIgnoredTasks As System.Windows.Forms.Button
    Friend WithEvents lblInstructions As System.Windows.Forms.Label
End Class
