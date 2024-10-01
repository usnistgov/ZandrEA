import React from 'react';
import PropTypes from 'prop-types';
import Typography from '@mui/material/Typography';

import FeatureList from './FeatureList';
import CaseList from './CaseList';
import RulekitList from './RulekitList';

const SubjectDetail = (props) => {
  const { subject } = props;

  return (
    <div>
    <Typography variant="h6" style={{ fontWeight: 600 }}>Features of Subject {subject.name}: {subject.label}</Typography>
    <FeatureList features={subject.features} /><br />
    <Typography variant="h6" style={{ fontWeight: 600 }}>Cases of Subject {subject.name}: {subject.label}</Typography><br />
    <CaseList cases={subject.cases} subject={subject.key} /><br />
    <Typography variant="h6" style={{ fontWeight: 600 }}>Rulekits of Subject {subject.name}: {subject.label}</Typography><br />
    <RulekitList rulekits={subject.rulekits} /><br />
    </div>
  );
};

SubjectDetail.propTypes = {
  subject: PropTypes.object.isRequired,
};

export default SubjectDetail;
