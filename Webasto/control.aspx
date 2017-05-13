<%@ Page Language="VB" AutoEventWireup="false" CodeFile="control.aspx.vb" Inherits="control" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>webasto</title>
    <meta name="viewport" content="width=device-width">
    
    <style type="text/css">
  			body { background-color: #ddd; }
  			#container { height: 100%; width: 100%; display: table; }
  			#inner { vertical-align: middle; display: table-cell; }
  			#gauge_div { width: 300px; margin: 0 auto; }
  			#gauge_div2 { width: 300px; margin: 0 auto; }
				</style>
    
    
    <script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js'></script>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>
<script type='text/javascript'>

  // set your channel id here
  var channel_id = 30761;
  // set your channel's read api key here if necessary
  var api_key = 'DFWEIVTUGWRJ8854';
  // maximum value for the gauge
  var max_gauge_value = 100;
  // name of the gauge
  var gauge_name = 'T,°C';

  // global variables
  var chart, chart2, charts, data;

  // load the google gauge visualization
  google.load('visualization', '1', {packages:['gauge']});
  google.setOnLoadCallback(initChart);

  // display the data
  function displayData(point) {
    data.setValue(0, 0, gauge_name);
    data.setValue(0, 1, point);
    chart.draw(data, options);
  }
  function displayData2(point) {
    data.setValue(0, 0, gauge_name);
    data.setValue(0, 1, point);
    chart2.draw(data, options);
  }

  // load the data
  function loadData() {
    // variable for the data point
    var p;
    var p2;

    // get the data from thingspeak
    $.getJSON('https://api.thingspeak.com/channels/' + channel_id + '/feed/last.json?api_key=' + api_key, function(data) {

      // get the data point
      p = data.field1;
      p2 = data.field2;

      // if there is a data point display it
      if (p) {        
        displayData(p);
      }
      if (p2) {        
        displayData2(p2);
      }

    });
  }

  // initialize the chart
  function initChart() {

    data = new google.visualization.DataTable();
    data.addColumn('string', 'Label');
    data.addColumn('number', 'Value');
    data.addRows(1);

    chart = new google.visualization.Gauge(document.getElementById('gauge_div'));
    chart2 = new google.visualization.Gauge(document.getElementById('gauge_div2'));
    options = {width: 300, height: 300, redFrom: 80, redTo: 120, yellowFrom:5, yellowTo: 80, minorTicks: 5, min: -30, max: 120};

    loadData();

    // load new data every 10 seconds
    setInterval('loadData()', 5000);
  }

</script>
    
    
    
    
</head>
<body>
    <form id="form1" runat="server">
    <div style="border: 1px solid #333333; margin-right: auto; margin-left: auto; text-align: center; background-color: #E0E0E0; width: 300px;">
        <br />
        Пароль:<br />
        <asp:TextBox ID="TextBoxPass" runat="server" Width="240px" 
            Font-Size="X-Large"></asp:TextBox>
        <br />
        <asp:Button ID="ButtonON" runat="server" Text="ON" Width="120px" 
            Font-Size="X-Large" />
        <asp:Button ID="ButtonOFF" runat="server" Text="OFF" Width="120px" 
            Font-Size="X-Large" />
        <br />
        Текущее состояние:
        <br />
        <asp:Button ID="ButtonStatus" runat="server" Text="Статус:" Width="240px" 
            Font-Size="X-Large" />            
        <br />
        <br />
    </div>
    </form>
    <p>
    <div id="container">
      <div id="inner">
        <div id="gauge_div"></div>
        <div id="gauge_div2"></div>
      </div>
    </div>
    </p>
    
</body>
</html>
