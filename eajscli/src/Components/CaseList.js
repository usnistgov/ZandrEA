//import React, { Component } from "react";
import PropTypes from "prop-types";

import makeStyles from '@mui/styles/makeStyles';
import Typography from '@mui/material/Typography';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';

import { Accordion, AccordionSummary, AccordionDetails } from "./Accordion";
import CaseDetail from "./CaseDetail";

const useStyles = makeStyles({
  heading: {
    //fontSize: theme.typography.pxToRem(15),
    //fontWeight: theme.typography.fontWeightRegular,
  },
  secondaryHeading: {
    //fontSize: theme.typography.pxToRem(15),
    //color: theme.palette.text.secondary,
  },
});

const CaseList = (props) => {
  const { cases, subject } = props;
  const classes = useStyles();

  let case_list = [];
  if (cases && cases.length > 0) {
    case_list = cases.map((c,i) =>
      <Accordion key={"acc-"+subject+"-"+i} TransitionProps={{ unmountOnExit: true }} >
        <AccordionSummary id={"panel-"+c.id+"-header"} aria-controls={"panel-"+c.id+"-content"} expandIcon={<ExpandMoreIcon />}>
          <Typography className={classes.heading}>{c.label}</Typography>
        </AccordionSummary>
        <AccordionDetails>
          <CaseDetail case={c} subject={subject} />
        </AccordionDetails>
      </Accordion>
    );
  }
  return (
    <div>{case_list}</div>
  );
}

CaseList.propTypes = {
  cases: PropTypes.array.isRequired,
  subject: PropTypes.number.isRequired,
};

export default CaseList;
