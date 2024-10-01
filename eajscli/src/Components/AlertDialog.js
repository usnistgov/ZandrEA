//import React from 'react';
import PropTypes from 'prop-types';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogActions from '@mui/material/DialogActions';
import Button from '@mui/material/Button';

import AlertList from './AlertList';

const AlertDialog = (props) => {
    const { alerts, closedalerts, setClosedalerts, alertDialogOpen, setAlertDialogOpen } = props;

    // close the dialog
    const closeDialog = () => setAlertDialogOpen(false);

    // event handler to close all alerts and the alert dialog
    const dismissAll = (e) => {
        setAlertDialogOpen(false);
        setClosedalerts(props.alerts.map(a => a.id));
    };

    // event handler to close the alert dialog
    const handleClose = (e) => {
        closeDialog();
    };

    return (
        <Dialog open={alertDialogOpen} onClose={handleClose}>
          <DialogContent>
            <AlertList alerts={alerts} closedalerts={closedalerts} setClosedalerts={setClosedalerts}/>
          </DialogContent>
          <DialogActions>
            {alerts.length > 0 ? <Button onClick={dismissAll}>Dismiss All</Button> : ""}
            <Button onClick={closeDialog}>Close</Button>
          </DialogActions>
        </Dialog>

    );
};

AlertDialog.propTypes = {
  alerts: PropTypes.arrayOf(PropTypes.object).isRequired,
  closedalerts: PropTypes.arrayOf(PropTypes.number).isRequired,
  setClosedalerts: PropTypes.func.isRequired,
  alertDialogOpen: PropTypes.bool.isRequired,
  setAlertDialogOpen: PropTypes.func.isRequired,
};

export default AlertDialog;
