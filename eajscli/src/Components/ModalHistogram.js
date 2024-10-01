// Render a settings icon that opens on click to a modal dialog box with
// a knoblist.
import React, { useState, useCallback } from "react";
import PropTypes from "prop-types";
import 'react-responsive-modal/styles.css';
import { Modal } from 'react-responsive-modal';
import BarChartIcon from "@mui/icons-material/BarChart";
import Button from "@mui/material/Button";

import Histogram from "./Histogram";

const bumpz = { overlay: { zindex: 999990 } };

const ModalHistogram = (props) => {
  const { histogram, caption, iconcaption, arialabel } = props;

  const [knobModalOpen, setKnobModalOpen] = useState(false);

  const onOpenModal = useCallback(() => {
    setKnobModalOpen(true);
  }, [setKnobModalOpen]);

  const onCloseModal = useCallback(() => {
    setKnobModalOpen(false);
  }, [setKnobModalOpen]);

  if (histogram) {
    return (
      <div>
        <Button variant="outlined" color="primary" aria-label={arialabel} onClick={onOpenModal} size="small" fontSize="small">{iconcaption}<BarChartIcon size="small" /></Button>
        {
          knobModalOpen ?
            <Modal open={knobModalOpen} onClose={onCloseModal} center closeOnEsc closeOnOverlayClick styles={bumpz} >
              <h3>{caption}</h3>
              <Histogram
                key={histogram.key}
                id={histogram.key}
                type={histogram.type}
                idtext={histogram.idtext}
                bartype={histogram.bartype}
                barcount={histogram.barcount}
                caption={histogram.caption}
                barheights={histogram.bar_heights}
                barlabels={histogram.bar_labels}
                leftbarvalue={histogram.left_bar_value}
                eachbarincr={histogram.each_bar_incr}
                rightbarvalue={histogram.right_bar_value}
                modeindex={histogram.mode}
                spanindex={histogram.span}
                modeoptions={histogram.mode_options}
                spanoptions={histogram.span_options}
                knobs={histogram.knobs}
              />
            </Modal>
          : null
        }
      </div>
    );
  } else {
    return null;
  };
};

ModalHistogram.propTypes = {
  iconcaption: PropTypes.string.isRequired,
  arialabel: PropTypes.string.isRequired,
  caption: PropTypes.string.isRequired,
  histogram: PropTypes.object.isRequired,
};

export default ModalHistogram;
