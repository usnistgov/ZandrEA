//import React, { useState, useCallback } from "react";
import 'react-responsive-modal/styles.css';
import PropTypes from "prop-types";
import Typography from "@mui/material/Typography";
import Grid from "@mui/material/Grid";
import Box from "@mui/material/Box";
import makeStyles from '@mui/styles/makeStyles';

import ModalHistogram from "./ModalHistogram";
import Knob from "./Knob.js";
import Krono from "./Krono.js";

const useStyles = makeStyles({
  table: {
    border: "1px solid black",
    borderCollapse: "collapse",
    "& thead": {
      backgroundColor: "lightgrey",
    },
    "& td,th": {
      border: "1px solid black",
    },
    width: "100%",
  },
  root: {
    //padding: '0 0',
    //maxWidth: '600px',
    //backgroundColor: theme.palette.background.paper,
  },
});

const Rulekit = (props) => {
  const { id, caption, knobs, histogram, krono, rulelabels, ruletexts_if, ruletexts_then, rulestates, ruleknobs, rulehistograms } = props;
  const classes = useStyles();

  let rows = [];
  for (let i = 0; i < rulelabels.length; i++) {
    const k = ruleknobs[i];
    const s = rulelabels[i];
    rows[i] =
      <tr key={(k.key * 100)+i}>
        <td>{rulelabels[i]}</td>
        <td>{ruletexts_if[i]}</td>
        <td>{ruletexts_then[i]}</td>
        <td>{rulestates[i]}</td>
        <td><ModalHistogram histogram={rulehistograms[i]} iconcaption="" caption={s} arialabel={s}/></td>
        <td><Knob key={k.key} id={k.key} label={k.label} type={k.type} range_min={k.range_min} range_max={k.range_max} defaultvalue={k.value} units={k.units} /></td>
      </tr>;
  }
  let knoblist = [];
  if (knobs instanceof Array && knobs.length > 0) {
    for (const k of knobs) {
      knoblist.push(
        <Knob key={k.key} id={k.key} label={k.label} type={k.type} range_min={k.range_min} range_max={k.range_max} defaultvalue={k.value} options={k.options} units={k.units} />
      );
    }
  }

  return (
    <div>
      <Typography variant="h6">{caption}</Typography>
      <Grid container justifyContent="center" alignItems="flex-start" spacing={2}>
        {knoblist.map((k,i) => (
          <Grid item xs key={'rk'+id+'_k'+k.key} >
            {k}
          </Grid>
        ))}
        <Grid item xs key={'rk'+id+'_h'+histogram.key}>
          <Box padding={1} align="center">
            <ModalHistogram caption={`RuleKit Histogram`} iconcaption={`Histogram`} arialabel={`Open RuleKit ${id} Histogram`} histogram={histogram} />
          </Box>
        </Grid>
      </Grid>
      <Krono id={krono.key} type={krono.type} caption={krono.caption} panes={krono.panes} timestamps={krono.timestamps} knobs={krono.knobs} />
      <br />
      <table className={classes.table}>
        <thead className={classes.thead}>
          <tr>
            <th>Rule</th>
            <th>If</th>
            <th>Then</th>
            <th>State</th>
            <th>Histo</th>
            <th>Knob</th>
          </tr>
        </thead>
        <tbody>
          { rows }
        </tbody>
      </table>
    </div>
  );
};

Rulekit.propTypes = {
  id: PropTypes.number.isRequired,
  caption: PropTypes.string.isRequired,
  knobs: PropTypes.arrayOf(PropTypes.object).isRequired,
  histogram: PropTypes.object.isRequired,
  krono: PropTypes.object.isRequired,
  rulelabels: PropTypes.arrayOf(PropTypes.string).isRequired,
  ruletexts_if: PropTypes.arrayOf(PropTypes.string).isRequired,
  ruletexts_then: PropTypes.arrayOf(PropTypes.string).isRequired,
  rulestates: PropTypes.arrayOf(PropTypes.string).isRequired,
  ruleknobs: PropTypes.arrayOf(PropTypes.object).isRequired,
  rulehistograms: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default Rulekit;
