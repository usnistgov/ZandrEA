// Render a settings icon that opens on click to a modal dialog box with
// a knoblist.
import React, { useState, useCallback } from "react";
import PropTypes from "prop-types";
import 'react-responsive-modal/styles.css';
import { Modal } from 'react-responsive-modal';
import SettingsIcon from "@mui/icons-material/Settings";
import Button from "@mui/material/Button";

import KnobList from "./KnobList";

const bumpz = { overlay: { zindex: 999999 } };

const ModalKnobList = (props) => {
  const { knobs, caption, iconcaption, arialabel } = props;

  const [knobModalOpen, setKnobModalOpen] = useState(false);

  const onOpenModal = useCallback(() => {
    setKnobModalOpen(true);
  }, [setKnobModalOpen]);

  const onCloseModal = useCallback(() => {
    setKnobModalOpen(false);
  }, [setKnobModalOpen]);

  if (knobs && knobs instanceof Array && knobs.length > 0) {
    return (
      <div>
        <Button variant="outlined" color="primary" aria-label={arialabel} onClick={onOpenModal} size="small" fontSize="small">{iconcaption}<SettingsIcon size="small" /></Button>
        {
          knobModalOpen ?
            <Modal open={knobModalOpen} onClose={onCloseModal} center closeOnEsc closeOnOverlayClick styles={bumpz} >
              <h3>{caption}</h3>
              <KnobList knobs={knobs} />
            </Modal>
          : null
        }
      </div>
    );
  } else {
    return null;
  };
};

ModalKnobList.propTypes = {
  iconcaption: PropTypes.string.isRequired,
  arialabel: PropTypes.string.isRequired,
  caption: PropTypes.string.isRequired,
  knobs: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default ModalKnobList;
