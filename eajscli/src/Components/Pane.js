//import React, { useState, useCallback } from "react";
//import ReactDOM from "react-dom";
import PropTypes from "prop-types";
import Typography from "@mui/material/Typography";

import ChartAnalog from './ChartAnalog';
import ChartStates from './ChartStates';

const fills = {
  rule: {
    invalid:        {fill: "red",     stroke: "black",    opacity: 0.5},
    unavailable:    {fill: "yellow",  stroke: "black",    opacity: 0.5},
    autoMode_fail:  {fill: "red",     stroke: "blue",     opacity: 1.0},
    autoMode_pass:  {fill: "green",   stroke: "blue",     opacity: 1.0},
    autoMode_skip:  {fill: "yellow",  stroke: "blue",     opacity: 1.0},
    caseMode_fail:  {fill: "red",     stroke: "cyan",     opacity: 1.0},
    caseMode_pass:  {fill: "green",   stroke: "cyan",     opacity: 1.0},
    caseMode_skip:  {fill: "yellow",  stroke: "cyan",     opacity: 1.0},
    idleMode_fail:  {fill: "red",     stroke: "magenta",  opacity: 0.3},
    idleMode_pass:  {fill: "green",   stroke: "magenta",  opacity: 0.3},
    idleMode_skip:  {fill: "yellow",  stroke: "magenta",  opacity: 0.3},
  },
  fact: {
    invalid:        {fill: "red",     stroke: "black",    opacity: 0.5},
    unavailable:    {fill: "yellow",  stroke: "black",    opacity: 0.5},
    false:          {fill: "white",   stroke: "lightblue",opacity: 1.0},
    true:           {fill: "blue",    stroke: "lightblue",opacity: 1.0},
  }
};

const symbols = {
  rule:  "square",
  fact:  "circle",
};

const cap = (s) => {
  return s.charAt(0).toUpperCase() + s.slice(1);
};

const Pane = (props) => {
  const { id, type, width, yunits, traces, timestamps, reply } = props;

  if (! id) {
    return <Typography>(Cannot render a null pane!)</Typography>;
  }
  if (reply.search(/^FAIL_/) >= 0) {
    return <Typography>(Unable to query {type} with id {id}: {reply}</Typography>;
  }
  const warnings = traces.map(t => { if (t.reply.search(/^FAIL_/) >= 0) { return <Typography>Warning: unable to retrieve trace data for {t.key}</Typography>; } else { return null; } });

  const argumentField = "x";
  const valueFields = traces.map(t => t.tag);
  let traceHistograms = [];
  for (let t of traces) {
    traceHistograms[t.tag] = t.histogram;
  };
  let panel = "";
  let data = [];
  let xmin, xmax;

  const panetype = type.replace(/^Pane_/, '');

  //console.log("Pane: %s with valueFields %o", panetype, valueFields);

  switch (panetype) {
    case "analog":
      // display a chart of up to 3 analog values over time
      let ymin2 = undefined;  // ymin and ymax from DLL are ranged for the full possible range, not the actual data range
      let ymax2 = undefined;
      for (let i = 0; i < traces[0].values.length; i++) {
        const ts = new Date(timestamps[i] * 1000);
        if (xmin === undefined) {
          xmin = ts;
        }
        xmax = ts;

        data[i] = { x: ts, v: true };
        for (const t of traces) {
          if (t.type === 'Trace_analog') {
            const v = t.values[i];
            const s = t.states[i];
            data[i][t.tag] = {y: v, u: (s === "Analog_unavailable"), i: (s === "Analog_invalid")};
            if (ymin2 === undefined || v < ymin2) { ymin2 = v; }
            if (ymax2 === undefined || v > ymax2) { ymax2 = v; }
            if (s !== "Analog_valid") {
              data[i].v = false;
            }
          } else { console.log("ERROR: %s cannot render %s", type, t.type); }
        }
      }
      if (data !== null && data.length > 0) {
        panel = <ChartAnalog pane={id} width={width} data={data} argumentField={argumentField} valueFields={valueFields} yunits={yunits} ymin={ymin2} ymax={ymax2} xmin={xmin} xmax={xmax} traceHistograms={traceHistograms}/>;
      } else {
        panel = <Typography>(insufficient data for {type} chart with {traces.length} traces)</Typography>
      }
      break;
    case "fact":
    case "rule":
      // display a set of states over time,
      for (let i = 0; i < traces[0].states.length; i++) {
        const ts = new Date(timestamps[i] * 1000);
        if (xmin === undefined) {
          xmin = ts;
        }
        xmax = ts;
        data[i] = { x: ts };
        let j = 0;
        for (const t of traces) {
          const tag = t.tag;
          const tracetype = t.type.replace(/^Trace_/, '');
          if (tracetype === panetype) {
            data[i][tag] = {y: 1.0, b: j, s: t.states[i].replace(/^(Fact|Rule)_/, '')};
          } else { console.log("ERROR: %s cannot render %s", type, t.type); }
          j++;
        }
      }
      //console.log("Pane_state calling ChartState with af=%s vf=%o data=%o", argumentField, valueFields, data);
      if (data !== null && data.length > 0) {
        panel = <ChartStates pane={id} width={width} data={data} name={cap(panetype)} argumentField={argumentField} valueFields={valueFields} stateField="s" fills={fills[panetype]} symbol={symbols[panetype]} traceHistograms={traceHistograms}/>;
      } else {
        panel = <Typography>(insufficient data for {type} chart with {traces.length} traces)</Typography>
      }
      break;
    default:
      panel = <Typography>(unimplemented pane type {type})</Typography>;
      break;
  }

  //console.log("Pane: render %d (%s) traces=%o timestamps=%o", id, type, traces, timestamps);
  return(<div>{(type !== 'Undefined') ? <div>{warnings}{panel}</div> : "(not defined)"}</div>);
};

Pane.propTypes = {
  id: PropTypes.number.isRequired,
  type: PropTypes.string.isRequired,
  width: PropTypes.number.isRequired,
  yunits: PropTypes.string.isRequired,
  ymin: PropTypes.oneOfType([PropTypes.number, PropTypes.oneOf(['NaN'])]),
  ymax: PropTypes.oneOfType([PropTypes.number, PropTypes.oneOf(['NaN'])]),
  traces: PropTypes.arrayOf(PropTypes.object).isRequired,
  timestamps: PropTypes.arrayOf(PropTypes.number).isRequired,
  reply: PropTypes.string.isRequired,
};

export default Pane;
