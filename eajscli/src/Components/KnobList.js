//import rp from "react-polymer";
import React from "react";
import PropTypes from "prop-types";
import List from "@mui/material/List";
import ListItem from "@mui/material/ListItem";

import Knob from "./Knob";

const KnobList = (props) => {
  const { knobs } = props;

  return (
    <List dense>
      {
        knobs.map(f =>
          <ListItem key={`li-${f.key}`}>
            <Knob key={`k-${f.key}`} id={f.key} label={f.label} type={f.type} range_min={f.range_min} range_max={f.range_max} options={f.options} defaultvalue={f.value} units={f.units} />
          </ListItem>
        )
      }
    </List>
  );
}

KnobList.propTypes = {
  knobs: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default KnobList;
