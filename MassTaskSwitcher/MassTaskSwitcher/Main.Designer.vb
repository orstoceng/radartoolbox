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
        Dim ListViewItem2 As System.Windows.Forms.ListViewItem = New System.Windows.Forms.ListViewItem(New String() {"Default Item Name", "Disabled", "", ""}, -1)
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
        Me.btnSelectTasksToLock = New System.Windows.Forms.Button()
        Me.lblInstructions = New System.Windows.Forms.Label()
        Me.btnEditSelected = New System.Windows.Forms.Button()
        Me.cbTsGuiType = New System.Windows.Forms.ComboBox()
        Me.btnDeleteSelected = New System.Windows.Forms.Button()
        Me.btnOpenTaskScheduler = New System.Windows.Forms.Button()
        Me.btnRevertAllChanges = New System.Windows.Forms.Button()
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
        ListViewItem2.StateImageIndex = 0
        Me.lvTasks.Items.AddRange(New System.Windows.Forms.ListViewItem() {ListViewItem2})
        Me.lvTasks.Location = New System.Drawing.Point(12, 77)
        Me.lvTasks.Name = "lvTasks"
        Me.lvTasks.Size = New System.Drawing.Size(621, 294)
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
        Me.btnDisableAll.Location = New System.Drawing.Point(129, 410)
        Me.btnDisableAll.Name = "btnDisableAll"
        Me.btnDisableAll.Size = New System.Drawing.Size(111, 26)
        Me.btnDisableAll.TabIndex = 2
        Me.btnDisableAll.Text = "Disable All"
        Me.btnDisableAll.UseVisualStyleBackColor = True
        '
        'btnClose
        '
        Me.btnClose.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.btnClose.Location = New System.Drawing.Point(522, 410)
        Me.btnClose.Name = "btnClose"
        Me.btnClose.Size = New System.Drawing.Size(111, 26)
        Me.btnClose.TabIndex = 2
        Me.btnClose.Text = "Close"
        Me.btnClose.UseVisualStyleBackColor = True
        '
        'btnEnableAll
        '
        Me.btnEnableAll.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnEnableAll.Location = New System.Drawing.Point(12, 410)
        Me.btnEnableAll.Name = "btnEnableAll"
        Me.btnEnableAll.Size = New System.Drawing.Size(111, 26)
        Me.btnEnableAll.TabIndex = 2
        Me.btnEnableAll.Text = "Enable All"
        Me.btnEnableAll.UseVisualStyleBackColor = True
        '
        'btnSelectTasksToLock
        '
        Me.btnSelectTasksToLock.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.btnSelectTasksToLock.Location = New System.Drawing.Point(475, 377)
        Me.btnSelectTasksToLock.Name = "btnSelectTasksToLock"
        Me.btnSelectTasksToLock.Size = New System.Drawing.Size(158, 26)
        Me.btnSelectTasksToLock.TabIndex = 2
        Me.btnSelectTasksToLock.Text = "Select Tasks to Lock"
        Me.btnSelectTasksToLock.UseVisualStyleBackColor = True
        '
        'lblInstructions
        '
        Me.lblInstructions.AutoSize = True
        Me.lblInstructions.Location = New System.Drawing.Point(9, 9)
        Me.lblInstructions.Name = "lblInstructions"
        Me.lblInstructions.Size = New System.Drawing.Size(419, 65)
        Me.lblInstructions.TabIndex = 3
        Me.lblInstructions.Text = resources.GetString("lblInstructions.Text")
        '
        'btnEditSelected
        '
        Me.btnEditSelected.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnEditSelected.Location = New System.Drawing.Point(129, 378)
        Me.btnEditSelected.Name = "btnEditSelected"
        Me.btnEditSelected.Size = New System.Drawing.Size(111, 26)
        Me.btnEditSelected.TabIndex = 4
        Me.btnEditSelected.Text = "Edit Selected"
        Me.btnEditSelected.UseVisualStyleBackColor = True
        '
        'cbTsGuiType
        '
        Me.cbTsGuiType.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.cbTsGuiType.FormattingEnabled = True
        Me.cbTsGuiType.Items.AddRange(New Object() {"Native (XP) GUI", "Windows 7-style GUI"})
        Me.cbTsGuiType.Location = New System.Drawing.Point(246, 382)
        Me.cbTsGuiType.Name = "cbTsGuiType"
        Me.cbTsGuiType.Size = New System.Drawing.Size(136, 21)
        Me.cbTsGuiType.TabIndex = 5
        '
        'btnDeleteSelected
        '
        Me.btnDeleteSelected.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnDeleteSelected.Location = New System.Drawing.Point(12, 378)
        Me.btnDeleteSelected.Name = "btnDeleteSelected"
        Me.btnDeleteSelected.Size = New System.Drawing.Size(111, 26)
        Me.btnDeleteSelected.TabIndex = 4
        Me.btnDeleteSelected.Text = "Delete Selected"
        Me.btnDeleteSelected.UseVisualStyleBackColor = True
        '
        'btnOpenTaskScheduler
        '
        Me.btnOpenTaskScheduler.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnOpenTaskScheduler.Location = New System.Drawing.Point(246, 410)
        Me.btnOpenTaskScheduler.Name = "btnOpenTaskScheduler"
        Me.btnOpenTaskScheduler.Size = New System.Drawing.Size(136, 26)
        Me.btnOpenTaskScheduler.TabIndex = 4
        Me.btnOpenTaskScheduler.Text = "Open Task Scheduler"
        Me.btnOpenTaskScheduler.UseVisualStyleBackColor = True
        '
        'btnRevertAllChanges
        '
        Me.btnRevertAllChanges.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left), System.Windows.Forms.AnchorStyles)
        Me.btnRevertAllChanges.Location = New System.Drawing.Point(388, 410)
        Me.btnRevertAllChanges.Name = "btnRevertAllChanges"
        Me.btnRevertAllChanges.Size = New System.Drawing.Size(128, 26)
        Me.btnRevertAllChanges.TabIndex = 6
        Me.btnRevertAllChanges.Text = "Revert All Changes"
        Me.btnRevertAllChanges.UseVisualStyleBackColor = True
        '
        'Main
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(645, 448)
        Me.Controls.Add(Me.btnRevertAllChanges)
        Me.Controls.Add(Me.cbTsGuiType)
        Me.Controls.Add(Me.btnOpenTaskScheduler)
        Me.Controls.Add(Me.btnDeleteSelected)
        Me.Controls.Add(Me.btnEditSelected)
        Me.Controls.Add(Me.lblInstructions)
        Me.Controls.Add(Me.btnClose)
        Me.Controls.Add(Me.btnSelectTasksToLock)
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
    Friend WithEvents btnSelectTasksToLock As System.Windows.Forms.Button
    Friend WithEvents lblInstructions As System.Windows.Forms.Label
    Friend WithEvents btnEditSelected As System.Windows.Forms.Button
    Friend WithEvents cbTsGuiType As System.Windows.Forms.ComboBox
    Friend WithEvents btnDeleteSelected As System.Windows.Forms.Button
    Friend WithEvents btnOpenTaskScheduler As System.Windows.Forms.Button
    Friend WithEvents btnRevertAllChanges As System.Windows.Forms.Button
End Class
