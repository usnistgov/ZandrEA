//import React, { Component } from "react";
import PropTypes from "prop-types";
import Box from "@mui/material/Box";

import InputSlider from "./InputSlider";
import InputSwitch from "./InputSwitch";
import InputSelect from "./InputSelect";

const Knob = (props) => {
  const { id, label, type, defaultvalue, range_min, range_max, options, units } = props;
  const dval = Number(defaultvalue);

  let control = null;
  switch (type) {
    case 'Knob_takesGuiFpnAsBoolean':
      control = <InputSwitch knob={id} label={label} defaultvalue={dval} />;
      break;
    case 'Knob_selectsGuiFpnFromList':
      control = <InputSelect knob={id} label={label} options={options} defaultvalue={dval} />;
      break;
    default:
      const rstp = (range_max - range_min) / 200.0;
      control = <InputSlider knob={id} label={label} type={type} min={range_min} max={range_max} step={rstp} defaultvalue={dval} units={units} />
      break;
  }
  return (<Box border={1}>{control}</Box>);
};

Knob.propTypes = {
  id:           PropTypes.number.isRequired,
  label:        PropTypes.string.isRequired,
  type:         PropTypes.string.isRequired,  // "Knob_takesGuiFpnAs(Float|Integer|Boolean)"
  defaultvalue: PropTypes.oneOfType([PropTypes.number, PropTypes.bool]).isRequired,
  range_min:    PropTypes.number,
  range_max:    PropTypes.number,
  options:      PropTypes.arrayOf(PropTypes.number),
  units:        PropTypes.string.isRequired,
};

export default Knob;
