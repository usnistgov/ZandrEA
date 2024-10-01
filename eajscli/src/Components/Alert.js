import React from "react";
import ListItem from "@mui/material/ListItem";
import ListItemText from "@mui/material/ListItemText";
import ListItemSecondaryAction from "@mui/material/ListItemSecondaryAction";
import IconButton from "@mui/material/IconButton";
import RemoveCircleIcon from "@mui/icons-material/RemoveCircle";
import withStyles from '@mui/styles/withStyles';

const style = (theme) => ({
   root: {
   }
});

function Alert(props) {
   const handleClick = (event) => {
      // Here, we call the parent's state setter with our id
      props.doclose(props.id);
    }
   return (
      <ListItem key={props.key} value={props.id} dense>
         <ListItemText primary={props.message} />
         <ListItemSecondaryAction>
            <IconButton edge="end" aria-label="remove" onClick={handleClick} size="large">
               <RemoveCircleIcon />
            </IconButton>
         </ListItemSecondaryAction>
      </ListItem>
   );
}

export default withStyles(style)(Alert);
