//import React from "react";
import PropTypes from "prop-types";
import Grid from "@mui/material/Grid";
import Box from "@mui/material/Box";
import withStyles from '@mui/styles/withStyles';

import Feature from "./Feature.js";

const style = (theme) => ({
  root: {
  },
  table: {
      //minWidth: 400,
  },
});

const FeatureList = (props) => {
  const { features } = props;
  return (
    <Grid container justifyContent="center" alignItems="flex-start" spacing={2}>
    {
      features.map((f,i) =>
        <Grid key={`feature-${i}`} item xs={2}>
          <Box border={1} alignItems="center" justifyContent="center" textAlign="center">
            <Feature key={f.key} id={f.key} label={f.label} units={f.units} message={f.message} state={f.state} type={f.type} knobs={f.knobs} histogram={f.histogram} />
          </Box>
        </Grid>
      )
    }
    </Grid>
  );
}

FeatureList.propTypes = {
  features: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default withStyles(style)(FeatureList);
