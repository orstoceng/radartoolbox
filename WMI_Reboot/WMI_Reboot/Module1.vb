Option Explicit On
Option Strict On
Imports System.Management

Module Module1

    Sub Main()
        Dim strComputer As String = "."
        Dim options As New System.Management.ConnectionOptions
        options.Impersonation = ImpersonationLevel.Impersonate
        options.EnablePrivileges = True
        Dim ms As New ManagementScope("\\" & strComputer & "\root\CIMV2", options)
        Dim q As New SelectQuery("SELECT * FROM Win32_OperatingSystem")
        Dim search As New ManagementObjectSearcher(ms, q)

        Console.Write("System will forcibly reboot in ")
        For sec As Integer = 10 To 1 Step -1
            Console.Write(sec.ToString & ".. ")
            Threading.Thread.Sleep(1000)
        Next

        ' enum each entry for Win32_OperatingSystem and call WMI reboot method
        Dim args() As Object = {6, 0}
        For Each info As ManagementObject In search.Get()
            info.InvokeMethod("Win32Shutdown", args)
            'Console.WriteLine(info.GetText(TextFormat.Mof))
        Next

    End Sub

End Module
