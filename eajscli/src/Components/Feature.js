import React, { Component } from "react";
//import ReactDOM from "react-dom";
import 'react-responsive-modal/styles.css';
import PropTypes from "prop-types";
import Typography from "@mui/material/Typography";
import Grid from "@mui/material/Grid";
import withStyles from '@mui/styles/withStyles';

import ModalKnobList from "./ModalKnobList";
import ModalHistogram from "./ModalHistogram";

const styles = (theme) => ({
  root: {
    //padding: '0 0',
    //maxWidth: '600px',
    //backgroundColor: theme.palette.background.paper,
  },
  U: { },
  N: {
      color: 'rgba(0,0,255,1)',     // blue
  },
  Y: {
      color: 'rgba(0,255,0,1)',     // green
  },
  F: {
      backgroundColor: 'rgba(0,255,0,0.2)',     // green
  },
  T: {
      backgroundColor: 'rgba(255,0,0,0.2)',     // red
  },
  I: {
      backgroundColor: 'rgba(255,255,0,0.2)',   // yellow
  },
  X: {
      backgroundColor: 'rgba(255,165,0,0.2)',   // orange
  },
});

class Feature extends Component {

  static propTypes = {
    id: PropTypes.number.isRequired,
    label: PropTypes.string.isRequired,
    units: PropTypes.string.isRequired,
    message: PropTypes.string.isRequired,
    state: PropTypes.string.isRequired,
    type: PropTypes.string.isRequired,
    knobs: PropTypes.arrayOf(PropTypes.object),
    histogram: PropTypes.object,
  }

  static defaultProps = {
    message: "",
    state: "X",
  }

  render() {
    const { classes } = this.props;
    let cell = "UNDEF";
    if (this.props.type === 'Feature_fact') {
      if (this.props.state === "T") {
        cell = "True "+this.props.message;
      } else {
        cell = "False "+this.props.message;
      }
    } else {
      cell = this.props.message+" "+this.props.units;
    }

    const caption = `Knobs for feature ${this.props.label}`;
    const histocaption = `Histogram for feature ${this.props.label}`;

    return (
      <div>
        <Typography style={{ fontWeight: 550 }}>{this.props.label}</Typography>
        <Typography variant="body2" classes={{ root: classes[this.props.state] }}>{cell}</Typography>
        <Grid container alignItems="center" justifyContent="center" spacing={1}>
          {this.props.knobs.length > 0 &&
            <Grid key={`f${this.props.id}-knobs`} item xs>
              <ModalKnobList caption={caption} iconcaption="" arialabel={caption} knobs={this.props.knobs} />
            </Grid>
          }
          {this.props.histogram &&
            <Grid key={`f${this.props.id}-histo`} item xs>
              <ModalHistogram histogram={this.props.histogram} caption={histocaption} iconcaption="" arialabel={histocaption} />
            </Grid>
          }
        </Grid>
      </div>
    );
  }
}

export default withStyles(styles)(Feature);
