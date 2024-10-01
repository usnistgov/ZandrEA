import React from 'react';
import makeStyles from '@mui/styles/makeStyles';
import Grid from '@mui/material/Grid';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
import Typography from '@mui/material/Typography';

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

export default function InputSelect(props) {
  const classes = useStyles();
  const { knob, label, options, defaultvalue } = props;
  const [value, setValue] = React.useState(defaultvalue === 0 ? "" : defaultvalue);

  const handleChange = event => {
    const v = event.target.value;
    if (v === undefined || v === "") {
      return;
    }
    setValue(v);
    console.log("Setting knob " + knob + " to value " + v);
    fetchJsonWithTimeout('/set/knob', {
      credentials: 'same-origin',
      method: 'PUT',
      body: JSON.stringify({
        key: knob,
        value: +v,
      }),
    })
    .then(data => {
      console.log('InputSelect PUT succeeded');
    })
    .catch(error => console.log("ERROR while setting knob %d to value %f: %s", knob, v, error));

  };

  const slug = label.toString().toLowerCase()
    .replace(/\s+/g, '-')
    .replace(/[^\w-]+/g, '')
    .replace(/--+/g, '-')
    .replace(/^-+/, '')
    .replace(/-+$/, '');

  const myid = 'inputselect-' + slug;

  return (
    <div className={classes.root}>
      <Typography id={myid} variant="body2" gutterBottom>
        {label}
      </Typography>
      <Grid container spacing={2} alignItems="center">
        <Grid item xs style={{ textAlign: "center"}} >
            <Select
              value={value}
              onChange={handleChange}
              width={200}
              inputProps={{
                name: slug,
                id: `${slug}-input`,
              }}
            >
              <MenuItem key={`${slug}-99`} aria-label="Choose a value" value=""><em>None</em></MenuItem>
              { options.map((s,i) => <MenuItem key={`${slug}-${i}`} value={s}>{s}</MenuItem>)}
            </Select>
        </Grid>
      </Grid>
    </div>
  );
}
