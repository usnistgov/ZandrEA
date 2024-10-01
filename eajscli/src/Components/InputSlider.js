import React from 'react';
import PropTypes from "prop-types";
import makeStyles from '@mui/styles/makeStyles';
import Grid from '@mui/material/Grid';
import Typography from '@mui/material/Typography';
import Slider from '@mui/material/Slider';
import TextField from '@mui/material/TextField';

import { fetchJsonWithTimeout } from '../rest.js';

const useStyles = makeStyles({
  root: {
    minWidth: 300, // 400
    height: 80,
  },
  input: {
    width: 96,
  },
});

function InputSlider(props) {
  const classes = useStyles();
  const { knob, label, type, min, max, defaultvalue, units } = props;
  const [value, setValue] = React.useState(defaultvalue);

  // NOTE: takes value argument because state won't get updated until next render!
  const submitValue = (v) => {
    console.log("Setting knob " + knob + " to value " + v);
    fetchJsonWithTimeout('/set/knob', {
      credentials: 'same-origin',
      method: 'PUT',
      body: JSON.stringify({
        key: knob,
        value: v,
      }),
    })
    .then(data => {
      console.log('InputSlider PUT succeeded');
    })
    .catch(error => console.log("ERROR while setting knob " + knob + " to value " + v + ": " + error));

  };

  const handleSliderChange = (event, newValue) => {
    setValue(newValue);
  };

  const handleSliderRelease = (event, newValue) => {
    setValue(newValue);
    submitValue(newValue);
  }

  const handleInputChange = event => {
    if (event.target.value !== '') {
      const v = Number(event.target.value);
      setValue(v);
      submitValue(v);  // submit update on every keypress?? -- needed for the step arrows to work properly
    } else {
      setValue('');
    }
  };

  const handleBlur = () => {
    if (value < min) {
      setValue(min);
      submitValue(min);
    } else if (value > max) {
      setValue(max);
      submitValue(max);
    } else {
      submitValue(value);
    }
  };

  const slug = label.toString().toLowerCase()
    .replace(/\s+/g, '-')
    .replace(/[^\w-]+/g, '')
    .replace(/--+/g, '-')
    .replace(/^-+/, '')
    .replace(/-+$/, '');

  const myid = 'inputslider-' + slug;

  const step = (type === "Knob_takesGuiFpnAsInteger") ? (1) : (max-min)/500.0;

  const inputProps = {
    step: step,
    min: min !== undefined ? min : -Infinity,
    max: max !== undefined ? max : Infinity,
    type: 'number',
    'aria-labelledby': myid,
  };

  return (
    <div className={classes.root}>
      <Typography id={myid} variant="body2" gutterBottom>
        {label} ({units})
      </Typography>
      <Grid container spacing={2} alignItems="center">
        <Grid item xs>
          {min}
        </Grid>
        <Grid item xs>
          { (min !== undefined && max !== undefined) ?
            <Slider
              //defaultValue={defaultvalue}
              value={typeof value === 'number' ? value : min}
              step={step}
              min={min !== undefined ? min : -Infinity}
              max={max !== undefined ? max : Infinity}
              onChange={handleSliderChange}
              onChangeCommitted={handleSliderRelease}
              aria-labelledby={myid}
            />
            :
            ''
          }
        </Grid>
        <Grid item xs>
          {max}
        </Grid>
        <Grid item>
          <TextField
            className={classes.input}
            value={value}
            margin="dense"
            hiddenLabel
            variant="filled"
            //defaultValue={defaultvalue}
            onChange={handleInputChange}
            onBlur={handleBlur}
            inputProps={inputProps}
          />
        </Grid>
      </Grid>
    </div>
  );
}

InputSlider.propTypes = {
  knob: PropTypes.number.isRequired,
  label: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  min: PropTypes.number,
  max: PropTypes.number,
  defaultvalue: PropTypes.number.isRequired,
  units: PropTypes.string.isRequired,
};

export default InputSlider;
