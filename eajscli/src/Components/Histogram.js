import React, { useState } from "react";
//import ReactDOM from "react-dom";
import PropTypes from "prop-types";
import Typography from "@mui/material/Typography";
import Grid from "@mui/material/Grid";
import Box from "@mui/material/Box";
import FormControl from "@mui/material/FormControl";
import InputLabel from "@mui/material/InputLabel";
import MenuItem from "@mui/material/MenuItem";
import Select from "@mui/material/Select";
import makeStyles from '@mui/styles/makeStyles';
import {
  VictoryChart, VictoryAxis, VictoryBar, VictoryTheme, VictoryTooltip,
} from 'victory';

import ModalKnobList from "./ModalKnobList";
import { fetchJsonWithTimeout } from '../rest.js';

//import Chart from './Chart';

const useStyles = makeStyles((theme) => ({
  formControl: {
    margin: theme.spacing(1),
    minWidth: 120,
  },
  selectEmpty: {
    marginTop: theme.spacing(2),
  },
}));

const Histogram = (props) => {
  const { id, type, idtext, bartype, barcount, caption, barheights, barlabels, leftbarvalue, eachbarincr, rightbarvalue, modeindex, spanindex, modeoptions, spanoptions, knobs } = props;
  const classes = useStyles();

  const [mode, setMode] = useState(modeindex);
  const [span, setSpan] = useState(spanindex);

  const handleChange = (event) => {
    const name = event.target.name;
    const v = event.target.value;
    if (v === undefined || v === "") {
      return;
    }
    switch (name) {
      case 'mode': setMode(v); break;
      case 'span': setSpan(v); break;
      default: console.log("ERROR: Histogram handleChange: unrecognized target %s", name); break;
    }
    console.log("Setting histogram %d %s to value %s", id, name, v);
    fetchJsonWithTimeout(`/set/histogram/${name}`, {
      credentials: 'same-origin',
      method: 'PUT',
      body: JSON.stringify({
        key: id,
        value: +v,
      }),
    })
    .then(data => {
      console.log('Histogram %d %s PUT %s succeeded', id, name, v);
    })
    .catch(error => console.log("ERROR while setting histogram %d %s to value %s: %s", id, name, v, error));
  };

  const myHeight = 200;
  const myWidth = 500;

  if (! id) {
    return null;
  }

  let panel = "(not defined)";

  const knobcaption = `Knobs for ${idtext}`;
  const captionpanel = caption.map((s,i) => {
    return (<Typography key={`cap-${i}`} style={{ fontWeight: i === 0 ? 500 : 400, }}>{s}</Typography>);
  });
  const selectorpanel =
    <Grid container align="center" spacing={2}>
      <Grid item xs style={{ display: "flex", alignItems: "center", justifyContent: "center" }}>
        <FormControl variant="outlined" className={classes.formControl}>
          <InputLabel id="mode-input-label">Mode</InputLabel>
          <Select
            value={mode}
            label="Mode"
            onChange={handleChange}
            labelId="mode-input-label"
            inputProps={{
              name: 'mode',
              id: 'mode-input',
            }}
          >
            <MenuItem key='mode-blank' value=""><em>Set Histogram Mode</em></MenuItem>
            { modeoptions.map((s,i) => <MenuItem key={`mode-${i}`} value={i}>{s}</MenuItem>)}
          </Select>
        </FormControl>
      </Grid>
      <Grid item xs style={{ display: "flex", alignItems: "center", justifyContent: "center" }}>
        <FormControl variant="outlined" className={classes.formControl}>
          <InputLabel id="span-input-label">Span</InputLabel>
          <Select
            value={span}
            label="Span"
            onChange={handleChange}
            labelId="span-input-label"
            inputProps={{
              name: 'span',
              id: 'span-input',
            }}
          >
            <MenuItem key='span-blank' value=""><em>Set Histogram Span</em></MenuItem>
            { spanoptions.map((s,i) => <MenuItem key={`span-${i}`} value={i}>{s}</MenuItem>)}
          </Select>
        </FormControl>
      </Grid>
      {
        (knobs && knobs.length > 0) ?
          <Grid item xs style={{ display: "flex", alignItems: "center", justifyContent: "center" }}>
            <FormControl variant="outlined" className={classes.formControl}>
              <ModalKnobList knobs={knobs} caption={knobcaption} iconcaption="Knobs " arialabel={knobcaption} />
            </FormControl>
          </Grid>
        :
          null
      }
    </Grid>;

  switch (type) {
    case 'Histogram_analog':
    case 'Histogram_fact':
    case 'Histogram_rule':
    case 'Histogram_ruleKit':
      const analog = (type === 'Histogram_analog' && bartype === 'HistogramBars_analogBins');
      if (analog) {
        const yMax = Math.max(...barheights);
        const domain = {
          x: [leftbarvalue, rightbarvalue],
          y: [0, Math.ceil(yMax)]
        };
        const data = barheights.map((y,i) => {
          return ({
            x: leftbarvalue + (i * eachbarincr),
            y: y,
          });
        });
        panel = (
          <div>
            {captionpanel}
            <VictoryChart
              domainPadding={10}
              domain={domain}
              theme={VictoryTheme.material}
              height={myHeight}
              width={myWidth}
            >
              <VictoryBar data={data} barWidth={eachbarincr} labelComponent={<VictoryTooltip/>} labels={({ datum }) => datum.y} />
              <VictoryAxis />
              <VictoryAxis dependentAxis tickValues={[...Array(yMax > 0 ? Math.ceil(yMax+0.999999) : 2).keys()]} />
            </VictoryChart>
            {selectorpanel}
          </div>
        );

      } else {    // not analog but STATE bins
        let categories;
        if (barlabels.length !== barcount) {    // If the label count is wrong, fake our own labels
          console.log("WARNING: %s %s: barlabel %o count %d != barcount %d", type, idtext, barlabels, barlabels.length, barcount);
          let a = [];
          for (let i = 0; i < barcount; i++) {
            a.push(`S-${i}`);
          }
          categories = { x: a };
        } else {
          categories = { x: barlabels };
        }
        const data = categories.x.map((l,i) => {
          return {
            x: l,
            y: barheights[i],
          }
        });
        panel = (
          <div>
            {captionpanel}
            <VictoryChart
              data={data}
              domainPadding={10}
              theme={VictoryTheme.material}
              height={myHeight}
              width={myWidth}
            >
              <VictoryBar
                data={data}
                categories={categories}
                labelComponent={<VictoryTooltip/>} labels={({ datum }) => datum.y.toFixed(2)}
              />
              <VictoryAxis />
              <VictoryAxis dependentAxis />
            </VictoryChart>
            {selectorpanel}
          </div>
        );
      }
      break;
    default:
      panel = <Typography>(unimplemented histogram {idtext} type {type})</Typography>;
      break;
  }

  return (
    <div>
      {panel}
    </div>
  );
};

Histogram.propTypes = {
  id: PropTypes.number.isRequired,
  type: PropTypes.string.isRequired,
  idtext: PropTypes.string.isRequired,
  bartype: PropTypes.string.isRequired,
  barcount: PropTypes.number.isRequired,
  caption: PropTypes.arrayOf(PropTypes.string).isRequired,
  barheights: PropTypes.arrayOf(PropTypes.number).isRequired,
  barlabels: PropTypes.arrayOf(PropTypes.string).isRequired,
  leftbarvalue: PropTypes.oneOfType([PropTypes.number, PropTypes.oneOf(['NaN'])]).isRequired,
  eachbarincr: PropTypes.oneOfType([PropTypes.number, PropTypes.oneOf(['NaN'])]).isRequired,
  rightbarvalue: PropTypes.oneOfType([PropTypes.number, PropTypes.oneOf(['NaN'])]).isRequired,
  modeindex: PropTypes.number.isRequired,
  spanindex: PropTypes.number.isRequired,
  modeoptions: PropTypes.arrayOf(PropTypes.string).isRequired,
  spanoptions: PropTypes.arrayOf(PropTypes.string).isRequired,
  knobs: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default Histogram;
