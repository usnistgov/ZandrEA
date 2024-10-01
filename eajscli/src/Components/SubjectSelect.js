import React from 'react';
import PropTypes from 'prop-types';
import TextField from '@mui/material/TextField';
import Badge from '@mui/material/Badge';
import Autocomplete from '@mui/material/Autocomplete';
import makeStyles from '@mui/styles/makeStyles';

const useStyles = makeStyles(theme => ({
  inputRoot: {
    backgroundColor: "gray",
    "& .MuiOutlinedInput-notchedOutline": {
      borderColor: "white"
    },
    "&:hover .MuiOutlinedInput-notchedOutline": {
      borderColor: "red"
    },
    "&.Mui-focused .MuiOutlinedInput-notchedOutline": {
      borderColor: "purple"
    }
  }
}));

const SubjectSelect = (props) => {
  const { subjects, casecounts, opensubjectkeys, autocompletevalue, setAutocompletevalue, handleAddSubjectTab } = props;

  const classes = useStyles();

  const handleInputChange = (event, value) => {
    setAutocompletevalue(value);
  };

  return (
    <Autocomplete
      classes={classes}
      inputValue={autocompletevalue}
      value={null}
      id="subject-selector"
      options={subjects} //.filter(s => s !== null)}  //.filter(s => (!opensubjectkeys.includes(s.key)))}
      getOptionLabel={option => option ? option.name : ''}
      renderOption={(props, option) => {
          if (option) {
	    //console.log('renderOption: props=%o option=%o', props, option);
            let cc = casecounts[option.key];
            return (
              <li {...props}>{option.name}&nbsp;&nbsp;&nbsp;<Badge badgeContent={cc} color="error"/></li>
            );
          }
          return(null);
        }
      }
      getOptionDisabled={option => option === null || opensubjectkeys.includes(option.key)}
      onChange={handleAddSubjectTab}
      onInputChange={handleInputChange}
      noOptionsText="(no more subjects)"
      selectOnFocus
      clearOnBlur
      handleHomeEndKeys
      size="small"
      style={{ width: 300, }}
      renderInput={(params) =>
        <TextField {...params}
          inputProps={{...params.inputProps, autoComplete: "new-password"}}
          InputLabelProps={{ style: {color: "white"} }}
          autoComplete="off"
          label="Find subject to add"
          variant="outlined"
          size="small"
        />
      }
    />
  );
};

SubjectSelect.propTypes = {
  autocompletevalue: PropTypes.string.isRequired,
  setAutocompletevalue: PropTypes.func.isRequired,
  handleAddSubjectTab: PropTypes.func.isRequired,
  subjects: PropTypes.arrayOf(PropTypes.object).isRequired,
  casecounts: PropTypes.arrayOf(PropTypes.number).isRequired,
  opensubjectkeys: PropTypes.arrayOf(PropTypes.number).isRequired,
};

export default SubjectSelect;
