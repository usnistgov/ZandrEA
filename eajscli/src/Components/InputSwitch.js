import React from 'react';
import makeStyles from '@mui/styles/makeStyles';
import Grid from '@mui/material/Grid';
import Switch from '@mui/material/Switch';
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

export default function InputSwitch(props) {
  const classes = useStyles();
  const { knob, label, defaultvalue } = props;
  const [value, setValue] = React.useState(defaultvalue);

  const handleChange = event => {
    let v = event.target.checked ? 1.0 : 0.0;
    setValue(v);
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
      console.log('InputSwitch PUT succeeded');
    })
    .catch(error => console.log("ERROR while setting knob %d to value %f: %s", knob, v, error));

  };

  const slug = label.toString().toLowerCase()
    .replace(/\s+/g, '-')
    .replace(/[^\w-]+/g, '')
    .replace(/--+/g, '-')
    .replace(/^-+/, '')
    .replace(/-+$/, '');

  const myid = 'inputswitch-' + slug;

  return (
    <div className={classes.root}>
      <Typography id={myid} variant="body2" gutterBottom>
        {label}
      </Typography>
      <Grid container spacing={2} alignItems="center">
        <Grid item xs style={{ textAlign: "right" }} >
          False
        </Grid>
        <Grid item xs style={{ textAlign: "center"}} >
          <Switch
            defaultValue={defaultvalue}
            checked={value > 0.5 ? true : false}
            onChange={handleChange}
            aria-labelledby={myid}
          />
        </Grid>
        <Grid item xs style={{ textAlign: "left" }} >
          True
        </Grid>
      </Grid>
    </div>
  );
}
