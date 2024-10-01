import React, { useState, useEffect } from "react";
//import ReactDOM from "react-dom";
import 'react-responsive-modal/styles.css';
import PropTypes from "prop-types";
import Paper from "@mui/material/Paper";
import Typography from "@mui/material/Typography";

import Pane from "./Pane";
import ModalKnobList from "./ModalKnobList";

const Krono = (props) => {
  const { id, type, caption, panes, timestamps, knobs } = props;

  const [width, setWidth] = useState(window.innerWidth);
  const updateWidth = (ev) => {
    setWidth(ev.target.innerWidth)
  };

  // useEffect replaces `componentDidMount` and others
  useEffect(() => {
    window.addEventListener('resize', updateWidth)

    // Removes listener on unmount
    return () => {
      window.removeEventListener('resize', updateWidth)
    }
  },[]);

  let pids = [];
  let panelist = [];
  for (const p of panes) {
    if (!(pids.includes(p.key))) {
      pids.push(p.key);
      panelist.push(
        <Pane key={`pane-${p.key}`} id={p.key} type={p.type} width={width} yunits={p.yunits} ymin={p.ymin} ymax={p.ymax} traces={p.traces} timestamps={timestamps} reply={p.reply} />
      );
    } else {
      console.log("Krono: removed duplicate pane %d", p.key);
    }
  }

  //console.log("Krono: render %o", props);
  return (type !== 'Undefined') ? (
    <div>
      <Typography variant="h6">{caption}</Typography>
      <ModalKnobList iconcaption="Krono Knobs:" caption={`Knobs for Krono ${id}:`} arialabel={`Krono ${id} Knobs`} knobs={knobs} />
      {(panes && panes instanceof Array && panes.length > 0) ?
        <div>
          <Paper style={{ backgroundColor: 'ivory', }} >
            {panelist}
          </Paper>
        </div>
        :
        null
      }
    </div>
  )
  :
  ( <div>(not defined)</div>)
  ;
};

Krono.propTypes = {
  id: PropTypes.number.isRequired,
  type: PropTypes.string.isRequired,
  caption: PropTypes.string.isRequired,
  panes: PropTypes.arrayOf(PropTypes.object).isRequired,
  timestamps: PropTypes.arrayOf(PropTypes.number).isRequired,
  knobs: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default Krono;
