<?php

require_once(drupal_get_path('module', 'twitter_integration') . '/twitter_integration.common.inc');

function twitter_integration_follow_charity_sponsor($charity_sponsor) {
  list($sponsor_nid, $charity_nid) = explode(',', $charity_sponsor);

  $charity_node = node_load($charity_nid);
  $sponsor_node = node_load($sponsor_nid);

  // @TODO: validation based on CCK Approved field!
  $donation_text = $charity_node->field_active_donation_text[0]['value'];
  $donation_qty = $charity_node->field_qty[0]['value'];
  $output = t('Your contribution will help @charity to raise @donation_qty @donation_text.',
    array(
      '@charity' => $charity_node->title,
      '@donation_qty' => $donation_qty,
      '@donation_text' => $donation_text,
    ));
  $output .= '<br />';

  global $user;

  $link_text = t('Yes, I want to help');
  $return_url = 'dofollow/'. $charity_sponsor .'/confirmed';

  if(!$user->uid) {
    $output = l($link_text, twitter_generate_authorizeUrl($return_url));
  }
  else {
    $output .= l($link_text, $return_url);
  }

  return $output;
}

function twitter_integration_follow_confirmed($charity_sponsor) {
  global $user;

  if(isset($_GET['oauth_token'])) {
    dpm('OAUTH Login');
    // we just loggedin through twitter OAUTH
    $account = twitter_return_load_user();
    $user_oauth_token = $account->twitter_token;
    twitter_user_save_oauth_token($account->uid, $account->twitter_token);
    $logged_in = user_external_login($account, array());
    drupal_set_message(t("Successfully logged in."));
  }
  elseif($user->uid && $user_oauth_token = twitter_user_load_oauth_token($user->uid)) {
    dpm('User already logged in');
  }
  else {
    return 'error, not logged in';
  }

  list($sponsor_nid, $charity_nid) = explode(',', $charity_sponsor);

  $charity_node = node_load($charity_nid);
  $sponsor_node = node_load($sponsor_nid);

  $charity_oauth_token = twitter_user_load_oauth_token($charity_node->uid);
  $sponsor_oauth_token = twitter_user_load_oauth_token($sponsor_node->uid);

  try {
    $twitterObj = new EpiTwitter(CONSUMER_KEY, CONSUMER_SECRET, $user_oauth_token->oauth_token,  $user_oauth_token->oauth_token_secret);
    $follow_response = $twitterObj->post(FRIENDSHIP_CREATE_URL, array('follow' => 'yes', 'screen_name' => $charity_oauth_token->screen_name));
    $follow_response = $twitterObj->post(FRIENDSHIP_CREATE_URL, array('follow' => 'yes', 'screen_name' => $sponsor_oauth_token->screen_name));

    db_query("INSERT IGNORE INTO {f2g_twitter_followers} (client_uid, sponsor_uid, charity_uid, timestamp) VALUES (%d, %d, %d, %d)",
      $user->uid, $sponsor_node->uid, $charity_node->uid, time());
    drupal_set_message(t("Donation successful!"));
    drupal_goto();
  }
  catch (Exception $e) {
    
  }


  //twitter_user_load_oauth_token
  return 'donation not successful';
}

function twitter_integration_twitter_login() {
  require_once(drupal_get_path('module', 'twitter_integration') . '/twitter_integration.common.inc');
  $authorize_url = twitter_generate_authorizeUrl('twitter-login/loggedin');
  return l(t('Use your twitter account to Login or Create and account.'), $authorize_url, array('external' => TRUE));
}

function twitter_integration_twitter_login_loggedin() {
  if(isset($_GET['oauth_token'])) {
    // we just loggedin through twitter OAUTH
    $account = twitter_return_load_user();
    $user_oauth_token = $account->twitter_token;
    twitter_user_save_oauth_token($account->uid, $account->twitter_token);
    $logged_in = user_external_login($account, array());
    drupal_set_message(t("Successfully logged in."));
  }
  else {
    return 'error: no oauth_token param';
  }

  drupal_set_message(t('Congratulations, you logged in!'));
  drupal_goto();
}

function store_twitter_credentials_callback() {
  global $user;


  if(isset($_GET['oauth_token'])) {
    $account = twitter_return_load_user();
    twitter_user_save($account->twitter_token);
    db_query("INSERT IGNORE INTO {f2g_twitter_tokens} (uid, oauth_token, oauth_token_secret) VALUES (%d, '%s', '%s')",
      $account->uid, $account->twitter_token->oauth_token, $account->twitter_token->oauth_token_secret);

    $user = $account;
  }
  elseif(!$user->uid) {
    twitter_goto_twitter_login('store-twitter-credentials');
    //drupal_goto('twitter-login', array('destination' => 'store-twitter-credentials'));
  }

  $user_oauth = twitter_user_load_oauth_token($user->uid);
  if($user_oauth) {
    dpm($user_oauth, '$user_oauth');
    try {
    $twitterObj = twitter_get_EpiTwitter($user_oauth->oauth_token, $user_oauth->oauth_token_secret);
    }
    catch(Exception $e) {
      dpm($e->getMessage(), 'eerror');
    }
    dpm($twitterObj, '$twitterObj');
    //$twitterInfo= $twitterObj->get_statusesFriends();
  }

  return 'store_twitter_credentials_callback';
}


function twitter_integration_stored_data_test_callback($user) {
  $user_oauth = twitter_user_load_oauth_token($user->uid);
  try {
    $twitterObj = twitter_get_EpiTwitter($user_oauth->oauth_token, $user_oauth->oauth_token_secret);
    $twitterObj->setToken($user_oauth->oauth_token, $user_oauth->oauth_token_secret);

    $response = $twitterObj->get_accountVerify_credentials();

    $friends_response = $twitterObj->get('/friends/ids.json', array('screen_name' => $response->screen_name));
    $followers_response = $twitterObj->get('/followers/ids.json', array('screen_name' => $response->screen_name));
    $friendships_response = $twitterObj->get('/friendships/outgoing.json', array('screen_name' => $response->screen_name));
//format
    dpm($response, '$response');
    dpm($friends_response, '$friends_response');
    dpm($followers_response, '$followers_response');
    dpm($friendships_response, '$friendships_response');

    dpm(twitter_user_friends($twitterObj, $response->screen_name), 'friends');
  }
  catch(Exception $e) {
    dpm($e->getMessage(), 'eerror');
  }

  return 'twitter_integration_stored_data_test_callback';
}