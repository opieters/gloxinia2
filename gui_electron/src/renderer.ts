// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// No Node.js APIs are available in this process unless
// nodeIntegration is set to true in webPreferences.
// Use preload.js to selectively enable features
// needed in the renderer process.

//import { SerialPort } from "electron";

const { SerialPort } = require('serialport');
const { MockBinding } = require('@serialport/binding-mock')
const { handle_received_message, SerialMessage, set_data_handler_fn, update_log_rx_message, sm_to_buffer, add_log_message } = require("./GMessage")
const { DelimiterParser } = require('@serialport/parser-delimiter')
const { fs } = require("fs");
const luxon = require("luxon");
const DateTime = luxon.DateTime;

//import { SerialPort } from 'serialport'
//import type { SerialPort as SPT } from 'serialport';
//const tableify = require('tableify');
var Chart = require("chart.js");
import 'chartjs-adapter-luxon';
var port : typeof SerialPort;
var parser : typeof DelimiterParser;

const plot_buffer_length = 100;

var partial_data : Uint8Array = new Uint8Array(0);
const start_byte = 0x01;
const stop_byte = 0x04;

const canvas = <HTMLCanvasElement> document.getElementById('myChart')
const ctx = canvas.getContext('2d');

interface Sample {
  readonly t: typeof DateTime,
  readonly v: number,
}
var data_frame = new Map<string, Array<Sample>>();
var plot_samples = [
  {v: 12, t: new Date("2022-05-02 12:07:22")},
  {v: 19, t: new Date("2022-05-02 12:07:25")},
  {v:  3, t: new Date("2022-05-02 12:07:28")},
  {v:  5, t: new Date("2022-05-02 12:07:31")},
  {v:  2, t: new Date("2022-05-02 12:07:34")},
  {v:  3, t: new Date("2022-05-02 12:07:37")},
]
var plotted_trace = "";



function ddh(sensor_name : string, sensor_data : number){
  const sample = {t: new Date(), v: sensor_data / 0x8000};
  if(data_frame.has(sensor_name)){
      data_frame.get(sensor_name).push(sample);
      if(data_frame.get(sensor_name).length > plot_buffer_length){
        data_frame.get(sensor_name).shift();
      }
  } else {
      data_frame.set(sensor_name, [sample]);

      // update keys in trace list
      const trace_list_selector = <HTMLSelectElement> document.getElementById("trace-list");
      const trace_el = document.createElement("option");
      trace_el.appendChild(document.createTextNode(sensor_name));
      trace_list_selector.appendChild(trace_el);
  }
  if(sensor_name == plotted_trace){
    updateData(myChart, sample);
  }
}
set_data_handler_fn(ddh);


function create_chart(label : string, data: any){
  myChart = new Chart(ctx, {
    type: 'line',
    data: {
        datasets: [{
            label: label,
            data: data,
            //tension: 0.1,
            //fill: false,
            borderColor: 'rgb(75, 192, 75)'
        }]
    },
    options: {
        parsing: {
          xAxisKey: "t",
          yAxisKey: "v",
        },
        scales: {
            y: {
                beginAtZero: true
            },
            x: {
              type: 'time',
              time: {
                displayFormats: {second: 'HH:mm:ss'},
                unit: "second"
              },
              min: data[0].t,
            },
        },
        plugins: {
          legend: {
            display: false,
          },
        },
        maintainAspectRatio: false,
    }
});
}
create_chart("Dummy data", plot_samples);

function updateChartTrace(label :string, data: any){
  myChart.data.datasets[0].label = label;
  myChart.data.datasets[0].data = structuredClone(data);
  myChart.options.scales.x.min = structuredClone(data[0].t);
  myChart.update();
}

document.querySelector('#plot-trace-btn').addEventListener('click', () => {
  plotted_trace = (<HTMLSelectElement> document.getElementById("trace-list")).value;
  updateChartTrace(plotted_trace, data_frame.get(plotted_trace));
});


const device_code_map : Record<string, number> = {
  "sylvatica": 0x36,
  "planalta": 0x37,
  "apds9306": 0x34,
  "sht35": 0x35,
  "relay_board": 0x3a,
  "li6400": 0x38,
  "growth_chaimber": 0x39
}

function update_status(new_status : string, indicator: string){
  const status_indicator = document.getElementById("current-status");
  const status_dot = document.getElementById("statusdot");
  status_dot.className = `status-dot ${indicator}`;
  status_indicator.textContent = new_status;
}
update_status("Not connected.", "waiting");

async function listSerialPorts() {
  await SerialPort.list().then((ports: string | any[], err: { message: string }) => {
    if(err) {
      document.getElementById('error').textContent = err.message;
      return;
    } else {
      document.getElementById('error').textContent = '';
    }

    if (ports.length === 0) {
      document.getElementById('error').textContent = 'No ports discovered';
    }

    const sel = <HTMLSelectElement>  document.getElementById("port-selector");
    for(var i = 0; i < ports.length; i++){
      if(ports[i].manufacturer){
        var option = document.createElement("option");
        option.text = ports[i].path
        sel.add(option)
      }
    }
  })
};

listSerialPorts();



function setButtonActions(){
  const port_selector = <HTMLSelectElement> document.getElementById("port-selector");

  port_selector.addEventListener('change', (event) => {
    openPort((<HTMLTextAreaElement> event.target).value);
  });
}

document.querySelector('#add-sensor').addEventListener('click', () => {
  // activate sensor on the connected node
  const message : typeof SerialMessage = {
    command: 0x1c,
    id: 0,
    extended_id: 0,
    data: new Uint8Array(2),
  }

  const sensor_type = <HTMLSelectElement> document.getElementById("sensor-selector");
  const sensor_address = <HTMLInputElement> document.getElementById("sensor-address");

  message.data.set([device_code_map[sensor_type.value], parseInt(sensor_address.value, 10)])
  
  port.write(sm_to_buffer(message));

  // output activation to the log
  update_log_rx_message(message);

  // add sensor to the list of connected sensors
  const sensor_list = document.getElementById("connected-sensors");
  const new_sensor = document.createElement("li");
  new_sensor.appendChild(document.createTextNode(`${sensor_type.value} at ${sensor_address.value}`));
  sensor_list.appendChild(new_sensor);
});

setButtonActions();


function process_incoming_data(data : Uint8Array){
  const new_data = new Uint8Array(data.length + partial_data.length + 1);
  new_data.set(partial_data);
  new_data.set(data, partial_data.length);
  new_data.set([stop_byte], partial_data.length + data.length);
  partial_data = new_data;

  while(partial_data.length > 7){
    while((partial_data.length > 0) && (partial_data[0] !== start_byte)){
      console.log("Incorrect start byte detected.");
      partial_data = partial_data.slice(1);
    }

    const command = partial_data[1];
    const id = partial_data[2];
    const extended_id = (partial_data[3] << 8) | partial_data[4];
    const length = partial_data[5];
    if(partial_data.length < (7+length)){
      return;
    }
    const mdata = (length === 0) ? new Uint8Array(0) : new Uint8Array(partial_data.slice(6,6+length));
    if(partial_data[partial_data.length-1] !== stop_byte){
      console.log("Incorrect stop byte detected.");
      return;
    }
    
    const sm : typeof SerialMessage = {
      data: mdata,
      command: command,
      id: id,
      extended_id: extended_id,
    };

    handle_received_message(sm);

    partial_data = partial_data.slice(7+length);
  }
}

function openPort(path : string) {
  if(port){
    port.close();
  }

  port = new SerialPort({
      path: path,
      baudRate: 500000,
  });

  update_status(`Connected to ${path}`, "connected");

  // Open errors will be emitted as an error event
  port.on('error', function(err : {message: string}) {
    console.log('Error: ', err.message)
  });


  parser = port.pipe(new DelimiterParser({ delimiter: '\x04' }))
  parser.on('data', process_incoming_data) // emits data after every '\x04'

  // Read data that is available but keep the stream in "paused mode"
  /*port.on('data', function () {

  });
  port.on('readable', function () {
    var data = port.read(1)[0]
    if(data !== start_byte){
      console.log("Incorrect start byte detected.");
      return;
    }
    const command = port.read(1)[0];
    const id = port.read(1)[0];
    var data = port.read(2);
    data = data[0] << 8 | data[1];
    const extended_id = data;
    const length = port.read(1)[0];
    const mdata = (length === 0) ? new Uint8Array(0) : new Uint8Array(port.read(length));
    data = port.read(1)[0];
    if(data !== stop_byte){
      console.log("Incorrect stop byte detected.");
      return;
    }
    
    const sm : typeof SerialMessage = {
      data: mdata,
      command: command,
      id: id,
      extended_id: extended_id,
    };

    handle_received_message(sm);
    console.log('Data:', sm);
  })*/

  // Switches the port into "flowing mode"
  /*port.on('data', function (data : ArrayBuffer) {
    console.log('Data:', data)
  })*/
  /*
  // Pipe the data into another stream (like a parser or standard out)
  const lineStream = port.pipe(new Readline())
  */
  // The open event is always emitted
  port.on('open', function() {
    add_log_message("Opened port. Receiving and sending messages now.");
  })
}

/*document.querySelector('#start_measurement').addEventListener('click', () => {
  const start_message : typeof SerialMessage = {
    command: 0x00,
    id: 0,
    extended_id: 0,
    data: new Uint8Array(0),
  }
  port.write(sm_to_buffer(start_message));
});*/

document.querySelector('#start_sensors').addEventListener('click', () => {
  const start_message : typeof SerialMessage = {
    command: 0x1a,
    id: 0,
    extended_id: 0,
    data: new Uint8Array(0),
  }
  port.write(sm_to_buffer(start_message));
});

document.querySelector('#reset_system').addEventListener('click', () => {
  const start_message : typeof SerialMessage = {
    command: 0x05,
    id: 0,
    extended_id: 0,
    data: new Uint8Array(0),
  }
  port.write(sm_to_buffer(start_message));
});




var myChart : typeof Chart;

function updateData(chart : typeof Chart, sample : Sample) {
  const datasets = chart.data.datasets
    while((datasets[0].length + 1) > plot_buffer_length){
      datasets.forEach((dataset : any) => {
          dataset.data.shift();
      });
    }

    datasets.forEach((dataset : any) => {
        dataset.data.push(sample);
    });
    myChart.options.scales.x.min = data_frame.get(plotted_trace)[0].t;
    chart.update();
}



