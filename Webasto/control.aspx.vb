Imports System.IO
Imports System
Imports System.Web
Partial Class control
    Inherits System.Web.UI.Page

    Protected Sub ButtonStatus_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles ButtonStatus.Click
        GetStatus()
    End Sub
    Private Sub GetStatus()
        If TextBoxPass.Text = "dron6606" Then
            Dim theFileName As String = Path.Combine(Context.Server.MapPath("~/Uploads"), "log.txt")
            Dim line As String
            Using sr As New StreamReader(theFileName)
                line = sr.ReadLine()
            End Using

            If Not (line Is Nothing) Then
                If line.Contains("dron_on") Then
                    ButtonStatus.Text = "Статус: ON"
                End If

                If line.Contains("dron_off") Then
                    ButtonStatus.Text = "Статус: OFF"
                End If
            End If
        Else
            ButtonStatus.Text = "Неверный пароль"
        End If
    End Sub

    Protected Sub ButtonON_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles ButtonON.Click
       If TextBoxPass.Text = "dron6606" Then
            Dim theFileName As String = Path.Combine(Context.Server.MapPath("~/Uploads"), "log.txt")
                Using sr As New StreamWriter(theFileName)
                    sr.WriteLine("dron_on")
                End Using
        End If
        GetStatus()
    End Sub

    Protected Sub ButtonOFF_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles ButtonOFF.Click
        If TextBoxPass.Text = "dron6606" Then
            Dim theFileName As String = Path.Combine(Context.Server.MapPath("~/Uploads"), "log.txt")
            Using sr As New StreamWriter(theFileName)
                sr.WriteLine("dron_off")
            End Using
        End If
        GetStatus()
    End Sub
End Class
