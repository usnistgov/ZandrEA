// List of Rulekits
import React from "react";
import { PropTypes } from "prop-types";

import makeStyles from '@mui/styles/makeStyles';
import Typography from '@mui/material/Typography';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';

import { Accordion, AccordionSummary, AccordionDetails } from "./Accordion";
import Rulekit from "./Rulekit";

const useStyles = makeStyles({
  root: {
    //width: '98%',
    //maxWidth: 360,
    //backgroundColor: theme.palette.background.paper,
  },
  heading: {
    //fontSize: 20,
  },
});

const RulekitList = (props) => {
  const { rulekits } = props;
  const classes = useStyles();
  let rk_list = [];
  if (rulekits && rulekits.length > 0) {
    rk_list = rulekits.map((r,i) =>
      <Accordion key={"acc-" + r.key + "-" + i} TransitionProps={{ unmountOnExit: true }} >
        <AccordionSummary id={"panel-" + r.key + "-header"} aria-controls={"panel-" + r.key + "-content"} expandIcon={<ExpandMoreIcon />}>
          <Typography className={classes.heading}>{r.caption}</Typography>
        </AccordionSummary>
        <AccordionDetails>
          <Rulekit id={r.key} caption={r.caption} knobs={r.knobs} histogram={r.histogram} krono={r.krono}
                    rulelabels={r.rulelabels} ruletexts_if={r.ruletexts_if} ruletexts_then={r.ruletexts_then}
                    rulestates={r.rulestates} ruleknobs={r.ruleknobs} rulehistograms={r.rulehistograms}
          />
        </AccordionDetails>
      </Accordion>
    );
  }
  return (
    <div>{rk_list}</div>
  );
};

RulekitList.propTypes = {
  children: PropTypes.node,
  rulekits: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default RulekitList;
