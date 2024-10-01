import React, {Component} from "react";
import withStyles from '@mui/styles/withStyles';
import PropTypes from "prop-types";
import Typography from "@mui/material/Typography";
import Button from "@mui/material/Button";
import List from "@mui/material/List";
import ListItem from "@mui/material/ListItem";

import Krono from "./Krono.js";

import { fetchJsonWithTimeout } from '../rest.js';

const styles = theme => ({
  root: {
    width: '100%',
    backgroundColor: theme.palette.background.paper,
  },
  button: {
    textTransform: "none"
  }
});


class CaseDetail extends Component {

  static propTypes = {
    case:   PropTypes.object.isRequired,
    subject:  PropTypes.number.isRequired,
  };

  htmlize = (s) => s.replace(/\n/g, '');

  submitResponse = (answer) => {
    console.log("Case " + this.props.case.key + " subject " + this.props.subject + ": option " + answer + " clicked!");
    fetchJsonWithTimeout('/ctrl/answercase', {
      method: 'PUT',
      body: JSON.stringify({
        subject: this.props.subject,
        case: this.props.case.key,
        answer: answer,
      }),
    })
    .then(data => {
      console.log('CaseDetail PUT succeeded');
    })
    .catch(error => console.log("ERROR while Answering case " + this.props.case.key + " subject " + this.props.subject + " with index " + answer + ": " + error));
  }

  render() {
    let i = 0;
    let report = this.props.case.report ? this.props.case.report.map(s => <Typography key={i++}>{s}</Typography>) : "";
    let prompt = this.props.case.prompt ? this.props.case.prompt.map(s => <Typography paragraph key={i++}>{s}</Typography>) : "";
    let optionbuttons = this.props.case.options
      ? this.props.case.options.map((o, index) =>
          <ListItem key={index + '-' + o}>
            <Button className={this.props.classes.button} size="small" variant="outlined" value={index} onClick={this.submitResponse.bind(this, index)}>
              {o}
            </Button>
          </ListItem>)
      : "";

    const krono = this.props.case.krono;

    return(
      <div key={this.props.case.key}>
        <Krono id={krono.key} type={krono.type} caption={krono.caption} panes={krono.panes} timestamps={krono.timestamps} knobs={krono.knobs} />
        <br />
        <Typography paragraph variant="body1">Case Resolution:</Typography>
        {report}
        {prompt}
        <List dense>
          {optionbuttons}
        </List>
      </div>
    );
  }
}

export default withStyles(styles)(CaseDetail);
