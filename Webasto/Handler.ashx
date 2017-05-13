<%@ WebHandler Language="VB" Class="Handler" %>
Imports System.IO
Imports System
Imports System.Web

Public Class Handler : Implements IHttpHandler
    
    Private Shared Sub SaveCommandStatus(ByVal context As HttpContext, ByVal commandFileName As String, ByVal statusFileName As String)
        If context.Request.QueryString("commad") = "on" Then
            Using sr As New StreamWriter(commandFileName)
                sr.WriteLine("dron_on")
            End Using
        ElseIf context.Request.QueryString("commad") = "off" Then
            Using sr As New StreamWriter(commandFileName)
                sr.WriteLine("dron_off")
            End Using
            
        End If
           
        If context.Request.QueryString("status") = "on" Then
            Using sr As New StreamWriter(statusFileName)
                sr.WriteLine("on")
            End Using
        ElseIf context.Request.QueryString("status") = "off" Then
            Using sr As New StreamWriter(statusFileName)
                sr.WriteLine("off")
            End Using
        End If
    End Sub
    
    Public Sub ProcessRequest(ByVal context As HttpContext) Implements IHttpHandler.ProcessRequest
                
        
        If context.Request.QueryString("pas") = "dron" Then
            Dim commandFileName As String = Path.Combine(context.Server.MapPath("~/Uploads"), "log.txt")
            Dim statusFileName As String = Path.Combine(context.Server.MapPath("~/Uploads"), "status.txt")
           
            SaveCommandStatus(context, commandFileName, statusFileName)
            
            Dim command As String = ""
            Dim status As String = ""
            For i As Integer = 0 To 60 'таймаут ~60сек
                Using sr As New StreamReader(commandFileName)
                    command = sr.ReadLine()
                End Using
                Using sr As New StreamReader(statusFileName)
                    status = sr.ReadLine()
                End Using
                
                If command.Contains("dron_on") And status.Contains("off") Then 'если значение изменилось, то выходим сразу
                    context.Response.ContentType = "text/plain"
                    context.Response.Clear()
                    context.Response.Write("webasto_on")
                    Return
                End If
                
                                           
                If command.Contains("dron_off") And status.Contains("on") Then 'если значение изменилось, то выходим сразу
                    context.Response.ContentType = "text/plain"
                    context.Response.Clear()
                    context.Response.Write("webasto_off")
                    Return
                End If
                Threading.Thread.Sleep(1000) 'Спим 1 сек
            Next
            
            'Значение не поменялось за 60 сек просто отправим его
            If command.Contains("dron_on") Then
                context.Response.ContentType = "text/plain"
                context.Response.Clear()
                context.Response.Write("webasto_on")
                Return
            End If
                
                                           
            If command.Contains("dron_off") Then
                context.Response.ContentType = "text/plain"
                context.Response.Clear()
                context.Response.Write("webasto_off")
                Return
            End If
            
        End If
        
        'отправим ошибку
        context.Response.ContentType = "text/plain"
        context.Response.Clear()
        context.Response.Write("webasto_error")
        
    End Sub
 
    Public ReadOnly Property IsReusable() As Boolean Implements IHttpHandler.IsReusable
        Get
            Return False
        End Get
    End Property

End Class