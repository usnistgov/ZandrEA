//import React, { useState } from "react";
import PropTypes from 'prop-types';
import List from "@mui/material/List";
import Typography from "@mui/material/Typography";

import Alert from "./Alert.js";

function AlertList(props) {
    const { alerts, closedalerts, setClosedalerts } = props;

    const addClosed = (x) => { setClosedalerts([...closedalerts, x]); };
    let alertsection;

    if (alerts.length > 0) {
        alertsection = (
            <div>
                <Typography variant="h6">Domain-wide Alerts</Typography>
                <List dense border={1} width="100%">
                    { alerts.map(a => <Alert key={a.id} id={a.id} message={a.message} doclose={addClosed} />) }
                </List>
            </div>
        );
    } else {
        alertsection = (
            <p>No alerts to display</p>
        );
    }

    return (
        <div>
            {alertsection}
        </div>
    );
}

AlertList.propTypes = {
    alerts: PropTypes.arrayOf(PropTypes.object).isRequired,
    closedalerts: PropTypes.arrayOf(PropTypes.number).isRequired,
    setClosedalerts: PropTypes.func.isRequired,
};

export default AlertList;
